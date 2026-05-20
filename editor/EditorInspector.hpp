#pragma once

#include "entt/entt.hpp"
#include "magic_enum.hpp"
#include "raylib.h"

#include "engine/CollisionLayers.hpp"

#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace sage::editor
{
    struct InspectorField
    {
        enum class Kind
        {
            Bool,
            Int,
            UInt,
            UInt64,
            Float,
            String,
            Vec2,
            Vec3,
            Color,
            Enum, // also used for CollisionLayer (dropdown sourced from a runtime list)
        };

        std::string label;
        Kind kind = Kind::Bool;
        void* data = nullptr;
        bool editable = true;

        // Populated only when kind == Enum.
        std::vector<std::string> enumOptions;
        std::function<std::size_t()> getEnumIndex;
        std::function<void(std::size_t)> setEnumIndex;
    };

    // Components implement either a templated `inspect()` (cereal-style) or a
    // non-templated `inspect(editor::ComponentInspector&)`. Use the typed
    // methods to record fields.
    class ComponentInspector
    {
        std::vector<InspectorField> fields_;

      public:
        void Bool(std::string label, bool& v, bool editable = true);
        void Int(std::string label, int& v, bool editable = true);
        void UInt(std::string label, unsigned int& v, bool editable = true);
        void UInt64(std::string label, std::uint64_t& v, bool editable = true);
        void Float(std::string label, float& v, bool editable = true);
        void String(std::string label, std::string& v, bool editable = true);
        void Vec2(std::string label, Vector2& v, bool editable = true);
        void Vec3(std::string label, Vector3& v, bool editable = true);
        void Color(std::string label, ::Color& v, bool editable = true);

        // Bespoke: dropdown sourced from GetCollisionLayers(). Reuses Enum rendering.
        void CollisionLayer(std::string label, sage::CollisionLayer& v, bool editable = true);

        template <class E>
        void Enum(std::string label, E& v, bool editable = true)
        {
            static_assert(std::is_enum_v<E>, "ComponentInspector::Enum requires an enum type");
            InspectorField f{
                .label = std::move(label),
                .kind = InspectorField::Kind::Enum,
                .data = &v,
                .editable = editable};
            constexpr auto entries = magic_enum::enum_entries<E>();
            f.enumOptions.reserve(entries.size());
            for (const auto& [val, name] : entries) f.enumOptions.emplace_back(name);
            f.getEnumIndex = [p = &v]() -> std::size_t { return magic_enum::enum_index(*p).value_or(0); };
            f.setEnumIndex = [p = &v](const std::size_t idx) {
                constexpr auto vals = magic_enum::enum_values<E>();
                if (idx < vals.size()) *p = vals[idx];
            };
            fields_.push_back(std::move(f));
        }

        [[nodiscard]] std::vector<InspectorField> Take() && { return std::move(fields_); }
    };

    struct InspectedComponent
    {
        std::string displayName;
        std::vector<InspectorField> fields;
    };

    class InspectorRegistry
    {
        struct Entry
        {
            std::string displayName;
            std::function<bool(const entt::registry&, entt::entity)> has;
            std::function<std::vector<InspectorField>(entt::registry&, entt::entity)> describe;
        };

        std::vector<Entry> entries_;

      public:
        template <class T>
        void Register(std::string displayName)
        {
            entries_.push_back(
                {std::move(displayName),
                 [](const entt::registry& r, const entt::entity e) {
                     return r.valid(e) && r.template any_of<T>(e);
                 },
                 [](entt::registry& r, const entt::entity e) {
                     ComponentInspector ci;
                     r.template get<T>(e).inspect(ci);
                     return std::move(ci).Take();
                 }});
        }

        [[nodiscard]] std::vector<InspectedComponent> Inspect(
            entt::registry& registry, entt::entity entity) const;
    };

    void RegisterDefaultInspectorComponents(InspectorRegistry& registry);
} // namespace sage::editor
