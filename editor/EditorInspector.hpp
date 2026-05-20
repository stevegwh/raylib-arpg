#pragma once

#include "entt/entt.hpp"
#include "magic_enum.hpp"
#include "raylib.h"

#include "engine/CollisionLayers.hpp"

#include <cstdint>
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

    namespace detail
    {
        template <class T>
        inline constexpr bool always_false_v = false;
    }

    // Component-side authors declare a templated `inspect(Inspector&)` method (or a free
    // `inspect(Inspector&, T&)` for foreign types) and inside it call `i.field(label, value)`
    // for each member. Dispatch mirrors cereal's archive overloads:
    //
    //   - Concrete overloads handle "leaf" types (primitives, raylib Vec2/3/Color, …) and
    //     record one InspectorField row per call.
    //   - The enum template handles any `std::is_enum_v<E>` as an Enum-kind row.
    //   - The composite template recurses into the value's own `inspect()` (member or ADL
    //     free function), pushing a label prefix and propagating editability. Labels of
    //     sub-fields become "<parent> <child>" (e.g. "Local Bounds Min").
    //
    // Concrete overloads always win over the templates via overload resolution.
    class ComponentInspector
    {
        std::vector<InspectorField> fields_;
        std::string labelPrefix_;
        bool editableScope_ = true;

        [[nodiscard]] std::string qualified(const std::string& label) const
        {
            if (labelPrefix_.empty()) return label;
            if (label.empty()) return labelPrefix_;
            return labelPrefix_ + " " + label;
        }

        void addLeaf(const InspectorField::Kind kind, std::string label, void* data, const bool editable)
        {
            fields_.push_back(
                {.label = qualified(label),
                 .kind = kind,
                 .data = data,
                 .editable = editable && editableScope_});
        }

      public:
        // --- Leaf overloads ------------------------------------------------------------
        void field(std::string label, bool& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::Bool, std::move(label), &v, ed);
        }
        void field(std::string label, int& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::Int, std::move(label), &v, ed);
        }
        void field(std::string label, unsigned int& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::UInt, std::move(label), &v, ed);
        }
        void field(std::string label, std::uint64_t& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::UInt64, std::move(label), &v, ed);
        }
        void field(std::string label, float& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::Float, std::move(label), &v, ed);
        }
        void field(std::string label, std::string& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::String, std::move(label), &v, ed);
        }
        void field(std::string label, Vector2& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::Vec2, std::move(label), &v, ed);
        }
        void field(std::string label, Vector3& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::Vec3, std::move(label), &v, ed);
        }
        void field(std::string label, ::Color& v, bool ed = true)
        {
            addLeaf(InspectorField::Kind::Color, std::move(label), &v, ed);
        }

        // Bespoke: dropdown sourced from GetCollisionLayers(). Reuses Enum rendering.
        void field(std::string label, sage::CollisionLayer& v, bool ed = true);

        // --- Enum template -------------------------------------------------------------
        template <class E>
            requires std::is_enum_v<E>
        void field(std::string label, E& v, bool ed = true)
        {
            InspectorField f{
                .label = qualified(label),
                .kind = InspectorField::Kind::Enum,
                .data = &v,
                .editable = ed && editableScope_};
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

        // --- Composite template --------------------------------------------------------
        template <class T>
        void field(std::string label, T& v, bool ed = true)
        {
            const auto savedPrefix = labelPrefix_;
            const bool savedScope = editableScope_;
            labelPrefix_ = qualified(label);
            editableScope_ = editableScope_ && ed;

            if constexpr (requires { v.inspect(*this); })
                v.inspect(*this);
            else if constexpr (requires { inspect(*this, v); })
                inspect(*this, v);
            else
                static_assert(
                    detail::always_false_v<T>,
                    "ComponentInspector::field: type has no leaf overload, member inspect(), or ADL inspect()");

            labelPrefix_ = savedPrefix;
            editableScope_ = savedScope;
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

// --- ADL-discoverable inspect overloads for raylib types ---------------------
// Pattern mirrors engine/raylib-cereal.hpp: free functions in global namespace
// so unqualified `inspect(i, value)` finds them via ADL when the user composes a
// raylib type inside their component's inspect().

template <class Inspector>
void inspect(Inspector& i, BoundingBox& bb)
{
    i.field("Min", bb.min);
    i.field("Max", bb.max);
}
