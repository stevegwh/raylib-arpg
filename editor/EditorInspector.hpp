#pragma once

#include "entt/entt.hpp"
#include "magic_enum.hpp"
#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

namespace sage::editor
{
    enum class InspectorFieldKind
    {
        Bool,
        SignedInteger,
        UnsignedInteger,
        FloatingPoint,
        String,
        Vector2,
        Vector3,
        Color,
        Enum,
        Unsupported
    };

    struct InspectorFieldOptions
    {
        bool readOnly = false;
        std::optional<double> min;
        std::optional<double> max;
        std::optional<double> step;
    };

    struct InspectorField
    {
        std::string label;
        InspectorFieldKind kind = InspectorFieldKind::Unsupported;
        std::type_index valueType = typeid(void);
        void* mutableValue = nullptr;
        std::shared_ptr<void> ownedValue;
        const void* value = nullptr;
        InspectorFieldOptions options;
        std::function<std::string()> textValue;
        std::function<bool(const std::string&, const InspectorFieldOptions&)> setTextValue;
        std::function<bool()> boolValue;
        std::function<void(bool)> setBoolValue;
        std::vector<std::string> componentLabels;
        std::function<std::string(std::size_t)> componentTextValue;
        std::function<bool(std::size_t, const std::string&, const InspectorFieldOptions&)> setComponentTextValue;
        std::vector<std::string> enumOptions;
        std::function<std::optional<std::size_t>()> enumIndexValue;
        std::function<void(std::size_t)> setEnumIndexValue;

        [[nodiscard]] bool IsMutable() const
        {
            return mutableValue != nullptr && !options.readOnly;
        }
    };

    namespace detail
    {
        template <class T>
        struct dependent_false : std::false_type
        {
        };

        template <class T>
        [[nodiscard]] consteval InspectorFieldKind FieldKindFor()
        {
            using Value = std::remove_cvref_t<T>;
            if constexpr (std::is_same_v<Value, bool>)
            {
                return InspectorFieldKind::Bool;
            }
            else if constexpr (std::is_integral_v<Value> && std::is_signed_v<Value>)
            {
                return InspectorFieldKind::SignedInteger;
            }
            else if constexpr (std::is_integral_v<Value> && std::is_unsigned_v<Value>)
            {
                return InspectorFieldKind::UnsignedInteger;
            }
            else if constexpr (std::is_floating_point_v<Value>)
            {
                return InspectorFieldKind::FloatingPoint;
            }
            else if constexpr (std::is_same_v<Value, std::string>)
            {
                return InspectorFieldKind::String;
            }
            else if constexpr (std::is_same_v<Value, Vector2>)
            {
                return InspectorFieldKind::Vector2;
            }
            else if constexpr (std::is_same_v<Value, Vector3>)
            {
                return InspectorFieldKind::Vector3;
            }
            else if constexpr (std::is_same_v<Value, Color>)
            {
                return InspectorFieldKind::Color;
            }
            else if constexpr (std::is_enum_v<Value>)
            {
                return InspectorFieldKind::Enum;
            }
            else
            {
                return InspectorFieldKind::Unsupported;
            }
        }

        inline std::string trimNumericText(const std::string& text)
        {
            const auto first = text.find_first_not_of(" \t\n\r");
            if (first == std::string::npos) return {};
            const auto last = text.find_last_not_of(" \t\n\r");
            return text.substr(first, last - first + 1);
        }

        inline double applyOptions(const double value, const InspectorFieldOptions& options)
        {
            double result = value;
            if (options.min) result = std::max(result, *options.min);
            if (options.max) result = std::min(result, *options.max);
            return result;
        }

        template <class T>
        [[nodiscard]] std::string FormatScalarValue(const T& value)
        {
            using Value = std::remove_cvref_t<T>;
            if constexpr (std::is_same_v<Value, std::string>)
            {
                return value;
            }
            else if constexpr (std::is_floating_point_v<Value>)
            {
                return std::format("{:.2f}", value);
            }
            else if constexpr (std::is_integral_v<Value> && !std::is_same_v<Value, bool>)
            {
                return std::to_string(value);
            }
            else if constexpr (std::is_same_v<Value, bool>)
            {
                return value ? "true" : "false";
            }
            else
            {
                return {};
            }
        }

        template <class T>
        bool AssignScalarValue(T& value, const std::string& text, const InspectorFieldOptions& options)
        {
            using Value = std::remove_cvref_t<T>;
            const auto trimmed = trimNumericText(text);
            try
            {
                if constexpr (std::is_same_v<Value, std::string>)
                {
                    value = text;
                    return true;
                }
                else if constexpr (std::is_same_v<Value, bool>)
                {
                    if (trimmed == "true" || trimmed == "1")
                    {
                        value = true;
                        return true;
                    }
                    if (trimmed == "false" || trimmed == "0")
                    {
                        value = false;
                        return true;
                    }
                    return false;
                }
                else if constexpr (std::is_floating_point_v<Value>)
                {
                    std::size_t parsedChars = 0;
                    const double parsed = std::stod(trimmed, &parsedChars);
                    if (parsedChars != trimmed.size()) return false;
                    value = static_cast<Value>(applyOptions(parsed, options));
                    return true;
                }
                else if constexpr (std::is_integral_v<Value> && std::is_signed_v<Value>)
                {
                    std::size_t parsedChars = 0;
                    const long long parsed = std::stoll(trimmed, &parsedChars);
                    if (parsedChars != trimmed.size()) return false;
                    const double clamped = std::clamp(
                        applyOptions(static_cast<double>(parsed), options),
                        static_cast<double>(std::numeric_limits<Value>::min()),
                        static_cast<double>(std::numeric_limits<Value>::max()));
                    value = static_cast<Value>(std::llround(clamped));
                    return true;
                }
                else if constexpr (std::is_integral_v<Value> && std::is_unsigned_v<Value>)
                {
                    if (!trimmed.empty() && trimmed.front() == '-') return false;
                    std::size_t parsedChars = 0;
                    const unsigned long long parsed = std::stoull(trimmed, &parsedChars);
                    if (parsedChars != trimmed.size()) return false;
                    const double clamped = std::clamp(
                        applyOptions(static_cast<double>(parsed), options),
                        0.0,
                        static_cast<double>(std::numeric_limits<Value>::max()));
                    value = static_cast<Value>(std::llround(clamped));
                    return true;
                }
            }
            catch (const std::exception&)
            {
            }
            return false;
        }

        inline bool AssignColorComponent(
            unsigned char& component, const std::string& text, const InspectorFieldOptions& options)
        {
            std::size_t parsedChars = 0;
            const auto trimmed = trimNumericText(text);
            try
            {
                const long long parsed = std::stoll(trimmed, &parsedChars);
                if (parsedChars != trimmed.size()) return false;
                const double clamped = std::clamp(applyOptions(static_cast<double>(parsed), options), 0.0, 255.0);
                component = static_cast<unsigned char>(std::llround(clamped));
                return true;
            }
            catch (const std::exception&)
            {
            }
            return false;
        }

        template <class T>
        void ConfigureFieldAccessors(InspectorField& field, T* mutableValue, const T* value)
        {
            using Value = std::remove_cvref_t<T>;
            if (!value) return;

            if constexpr (std::is_same_v<Value, bool>)
            {
                field.boolValue = [value]() { return *value; };
                if (mutableValue)
                {
                    field.setBoolValue = [mutableValue](const bool newValue) { *mutableValue = newValue; };
                }
            }
            else if constexpr (std::is_same_v<Value, Vector2>)
            {
                field.componentLabels = {"X", "Y"};
                field.componentTextValue = [value](const std::size_t index) {
                    if (index == 0) return FormatScalarValue(value->x);
                    if (index == 1) return FormatScalarValue(value->y);
                    return std::string{};
                };
                if (mutableValue)
                {
                    field.setComponentTextValue = [mutableValue](
                                                      const std::size_t index,
                                                      const std::string& text,
                                                      const InspectorFieldOptions& options) {
                        if (index == 0) return AssignScalarValue(mutableValue->x, text, options);
                        if (index == 1) return AssignScalarValue(mutableValue->y, text, options);
                        return false;
                    };
                }
            }
            else if constexpr (std::is_same_v<Value, Vector3>)
            {
                field.componentLabels = {"X", "Y", "Z"};
                field.componentTextValue = [value](const std::size_t index) {
                    if (index == 0) return FormatScalarValue(value->x);
                    if (index == 1) return FormatScalarValue(value->y);
                    if (index == 2) return FormatScalarValue(value->z);
                    return std::string{};
                };
                if (mutableValue)
                {
                    field.setComponentTextValue = [mutableValue](
                                                      const std::size_t index,
                                                      const std::string& text,
                                                      const InspectorFieldOptions& options) {
                        if (index == 0) return AssignScalarValue(mutableValue->x, text, options);
                        if (index == 1) return AssignScalarValue(mutableValue->y, text, options);
                        if (index == 2) return AssignScalarValue(mutableValue->z, text, options);
                        return false;
                    };
                }
            }
            else if constexpr (std::is_same_v<Value, Color>)
            {
                field.componentLabels = {"R", "G", "B", "A"};
                field.componentTextValue = [value](const std::size_t index) {
                    if (index == 0) return std::to_string(static_cast<int>(value->r));
                    if (index == 1) return std::to_string(static_cast<int>(value->g));
                    if (index == 2) return std::to_string(static_cast<int>(value->b));
                    if (index == 3) return std::to_string(static_cast<int>(value->a));
                    return std::string{};
                };
                if (mutableValue)
                {
                    field.setComponentTextValue = [mutableValue](
                                                      const std::size_t index,
                                                      const std::string& text,
                                                      const InspectorFieldOptions& options) {
                        if (index == 0) return AssignColorComponent(mutableValue->r, text, options);
                        if (index == 1) return AssignColorComponent(mutableValue->g, text, options);
                        if (index == 2) return AssignColorComponent(mutableValue->b, text, options);
                        if (index == 3) return AssignColorComponent(mutableValue->a, text, options);
                        return false;
                    };
                }
            }
            else if constexpr (std::is_enum_v<Value>)
            {
                constexpr auto entries = magic_enum::enum_entries<Value>();
                field.enumOptions.reserve(entries.size());
                for (const auto& [enumValue, enumName] : entries)
                {
                    field.enumOptions.emplace_back(enumName);
                }
                field.enumIndexValue = [value]() { return magic_enum::enum_index(*value); };
                if (mutableValue)
                {
                    field.setEnumIndexValue = [mutableValue](const std::size_t index) {
                        constexpr auto values = magic_enum::enum_values<Value>();
                        if (index < values.size()) *mutableValue = values[index];
                    };
                }
            }
            else
            {
                field.textValue = [value]() { return FormatScalarValue(*value); };
                if (mutableValue)
                {
                    field.setTextValue =
                        [mutableValue](const std::string& text, const InspectorFieldOptions& options) {
                            return AssignScalarValue(*mutableValue, text, options);
                        };
                }
            }
        }
    } // namespace detail

    class Inspector
    {
      public:
        class FieldEditor
        {
            InspectorField* field = nullptr;

          public:
            explicit FieldEditor(InspectorField* _field) : field(_field)
            {
            }

            FieldEditor& readOnly()
            {
                if (field) field->options.readOnly = true;
                return *this;
            }

            FieldEditor& min(const double value)
            {
                if (field) field->options.min = value;
                return *this;
            }

            FieldEditor& max(const double value)
            {
                if (field) field->options.max = value;
                return *this;
            }

            FieldEditor& step(const double value)
            {
                if (field) field->options.step = value;
                return *this;
            }

            FieldEditor& range(const double minValue, const double maxValue)
            {
                return min(minValue).max(maxValue);
            }
        };

        template <class T>
        FieldEditor field(std::string label, T& value)
        {
            auto field = InspectorField{
                .label = std::move(label),
                .kind = detail::FieldKindFor<T>(),
                .valueType = typeid(std::remove_cvref_t<T>),
                .mutableValue = &value,
                .value = &value};
            detail::ConfigureFieldAccessors(field, &value, &value);
            fields.push_back(std::move(field));
            return FieldEditor{&fields.back()};
        }

        template <class T>
        FieldEditor readOnlyField(std::string label, const T& value)
        {
            auto ownedValue = std::make_shared<std::remove_cvref_t<T>>(value);
            auto field = InspectorField{
                .label = std::move(label),
                .kind = detail::FieldKindFor<T>(),
                .valueType = typeid(std::remove_cvref_t<T>),
                .mutableValue = nullptr,
                .ownedValue = ownedValue,
                .value = ownedValue.get(),
                .options = {.readOnly = true}};
            detail::ConfigureFieldAccessors(
                field, static_cast<std::remove_cvref_t<T>*>(nullptr), ownedValue.get());
            fields.push_back(std::move(field));
            return FieldEditor{&fields.back()};
        }

        [[nodiscard]] const std::vector<InspectorField>& GetFields() const
        {
            return fields;
        }

        std::vector<InspectorField> TakeFields()
        {
            return std::move(fields);
        }

      private:
        std::vector<InspectorField> fields;
    };

    template <class Component>
    concept MemberInspectable =
        requires(Component& component, Inspector& inspector) { component.inspect(inspector); };

    struct InspectedComponent
    {
        std::string displayName;
        std::type_index componentType = typeid(void);
        std::vector<InspectorField> fields;
    };

    class ComponentInspectorRegistry
    {
      public:
        template <class Component>
        void Register(std::string displayName)
        {
            Register<Component>(std::move(displayName), [](Component& component, Inspector& inspector) {
                if constexpr (MemberInspectable<Component>)
                {
                    component.inspect(inspector);
                }
            });
        }

        template <class Component, class InspectFn>
        void Register(std::string displayName, InspectFn&& inspectFn)
        {
            entries.push_back(
                Entry{
                    .componentType = typeid(Component),
                    .displayName = std::move(displayName),
                    .hasComponent =
                        [](const entt::registry& registry, const entt::entity entity) {
                            return registry.valid(entity) && registry.any_of<Component>(entity);
                        },
                    .inspect = [fn = std::forward<InspectFn>(inspectFn)](
                                   entt::registry& registry, const entt::entity entity) -> InspectedComponent {
                        Inspector inspector;
                        fn(registry.get<Component>(entity), inspector);
                        return {
                            .displayName = std::string{},
                            .componentType = typeid(Component),
                            .fields = inspector.TakeFields()};
                    }});
        }

        [[nodiscard]] std::vector<InspectedComponent> InspectEntity(
            entt::registry& registry, entt::entity entity) const;

      private:
        struct Entry
        {
            std::type_index componentType = typeid(void);
            std::string displayName;
            std::function<bool(const entt::registry&, entt::entity)> hasComponent;
            std::function<InspectedComponent(entt::registry&, entt::entity)> inspect;
        };

        std::vector<Entry> entries;
    };

    void RegisterDefaultInspectorComponents(ComponentInspectorRegistry& registry);
} // namespace sage::editor
