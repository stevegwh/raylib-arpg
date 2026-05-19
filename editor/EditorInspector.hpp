#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <concepts>
#include <cstdint>
#include <functional>
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
        const void* value = nullptr;
        InspectorFieldOptions options;

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
            fields.push_back(
                InspectorField{
                    .label = std::move(label),
                    .kind = detail::FieldKindFor<T>(),
                    .valueType = typeid(std::remove_cvref_t<T>),
                    .mutableValue = &value,
                    .value = &value});
            return FieldEditor{&fields.back()};
        }

        template <class T>
        FieldEditor readOnlyField(std::string label, const T& value)
        {
            fields.push_back(
                InspectorField{
                    .label = std::move(label),
                    .kind = detail::FieldKindFor<T>(),
                    .valueType = typeid(std::remove_cvref_t<T>),
                    .mutableValue = nullptr,
                    .value = &value,
                    .options = {.readOnly = true}});
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
    concept MemberInspectable = requires(Component& component, Inspector& inspector) {
        component.inspect(inspector);
    };

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
            Register<Component>(
                std::move(displayName),
                [](Component& component, Inspector& inspector) {
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
                    .inspect =
                        [fn = std::forward<InspectFn>(inspectFn)](
                            entt::registry& registry,
                            const entt::entity entity) -> InspectedComponent {
                            Inspector inspector;
                            fn(registry.get<Component>(entity), inspector);
                            return {
                                .displayName = std::string{},
                                .componentType = typeid(Component),
                                .fields = inspector.TakeFields()};
                        }});
        }

        [[nodiscard]] std::vector<InspectedComponent> InspectEntity(
            entt::registry& registry,
            entt::entity entity) const;

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
