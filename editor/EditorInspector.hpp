#pragma once

#include "entt/entt.hpp"
#include "magic_enum.hpp"
#include "raylib.h"

#include "engine/CollisionLayers.hpp"
#include "engine/components/sgTransform.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace sage::editor
{
    template <class T>
    struct LeafField
    {
        T* data = nullptr;
        std::function<void(const T&)> setter;
    };

    // Holds the data + options + index getter/setter for a dropdown-rendered field.
    // Used by both std::is_enum_v<E> values (options derived from magic_enum) and the
    // bespoke CollisionLayer overload (options derived from GetCollisionLayers()).
    struct EnumField
    {
        void* data = nullptr;
        std::vector<std::string> options;
        std::function<std::size_t()> getIndex;
        std::function<void(std::size_t)> setIndex;
    };

    // The variant alternative *is* the kind. Leaf overloads on ComponentInspector
    // construct one alternative each; the renderer dispatches via std::visit into
    // overloaded createFieldView/Update functions.
    using FieldValue = std::variant<
        LeafField<bool>,
        LeafField<int>,
        LeafField<unsigned int>,
        LeafField<std::uint64_t>,
        LeafField<float>,
        LeafField<std::string>,
        LeafField<Vector2>,
        LeafField<Vector3>,
        LeafField<::Color>,
        EnumField>;

    struct InspectorField
    {
        std::string label;
        bool editable = true;
        FieldValue value;
    };

    namespace detail
    {
        template <class T>
        inline constexpr bool always_false_v = false;
    }

    // Component-side authors declare a templated `define_editor_fields(Inspector&)` method
    // (or a free `define_editor_fields(Inspector&, T&)` for foreign types) and inside it
    // call `i.field(label, value)` for each member. Dispatch mirrors cereal's archive overloads:
    //
    //   - Concrete overloads handle "leaf" types (primitives, raylib Vec2/3/Color, …) and
    //     record one InspectorField row per call.
    //   - The enum template handles any `std::is_enum_v<E>` as an Enum-kind row.
    //   - The composite template recurses into the value's own `define_editor_fields()`
    //     (member or ADL free function), pushing a label prefix and propagating editability.
    //     Labels of sub-fields become "<parent> <child>" (e.g. "Local Bounds Min").
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

        template <class T>
        void addLeaf(std::string label, T* data, const bool editable)
        {
            fields_.push_back(
                {.label = qualified(label), .editable = editable && editableScope_, .value = LeafField<T>{data}});
        }

        template <class T>
        void addLeaf(std::string label, T* data, std::function<void(const T&)> setter)
        {
            fields_.push_back(
                {.label = qualified(label),
                 .editable = editableScope_,
                 .value = LeafField<T>{.data = data, .setter = std::move(setter)}});
        }

      public:
        // --- Leaf overloads ------------------------------------------------------------
        void field(std::string label, bool& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, bool& v, std::function<void(const bool&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, int& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, int& v, std::function<void(const int&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, unsigned int& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, unsigned int& v, std::function<void(const unsigned int&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, std::uint64_t& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, std::uint64_t& v, std::function<void(const std::uint64_t&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, float& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, float& v, std::function<void(const float&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, std::string& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, std::string& v, std::function<void(const std::string&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, Vector2& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, Vector2& v, std::function<void(const Vector2&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, Vector3& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, Vector3& v, std::function<void(const Vector3&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }
        void field(std::string label, ::Color& v, bool rw = true)
        {
            addLeaf(std::move(label), &v, rw);
        }
        void field(std::string label, ::Color& v, std::function<void(const ::Color&)> setter)
        {
            addLeaf(std::move(label), &v, std::move(setter));
        }

        // Bespoke: dropdown sourced from GetCollisionLayers(). Stored as EnumField.
        void field(const std::string& label, sage::CollisionLayer& v, bool rw = true);

        // sgTransform proxy. Reads come from the cached Vector3 inside the proxy;
        // writes route through the proxy's operator=, which dispatches to TransformSystem
        // so the hierarchy stays in sync. The component author just passes the field,
        // no setter lambda required.
        template <auto Write>
        void field(std::string label, ::sage::sgTransform::VectorField<Write>& proxy, bool rw = true)
        {
            // `data` points at the cached Vector3 inside the proxy (used for display only).
            // The const_cast is safe because the setter is always provided for proxy fields;
            // commitField uses the setter, not the data pointer, for writes.
            auto* data = const_cast<Vector3*>(&proxy.Get());
            if (!rw || !editableScope_)
            {
                addLeaf(std::move(label), data, false);
                return;
            }
            addLeaf(
                std::move(label),
                data,
                std::function<void(const Vector3&)>([&proxy](const Vector3& v) { proxy = v; }));
        }

        // --- Enum template -------------------------------------------------------------
        template <class E>
            requires std::is_enum_v<E>
        void field(std::string label, E& v, bool rw = true)
        {
            EnumField e{.data = &v};
            constexpr auto entries = magic_enum::enum_entries<E>();
            e.options.reserve(entries.size());
            for (const auto& [val, name] : entries)
                e.options.emplace_back(name);
            e.getIndex = [p = &v]() -> std::size_t { return magic_enum::enum_index(*p).value_or(0); };
            e.setIndex = [p = &v](const std::size_t idx) {
                constexpr auto vals = magic_enum::enum_values<E>();
                if (idx < vals.size()) *p = vals[idx];
            };
            fields_.push_back(
                {.label = qualified(label), .editable = rw && editableScope_, .value = std::move(e)});
        }

        // --- Composite template --------------------------------------------------------
        template <class T>
        void field(std::string label, T& v, bool rw = true)
        {
            const auto savedPrefix = labelPrefix_;
            const bool savedScope = editableScope_;
            labelPrefix_ = qualified(label);
            editableScope_ = editableScope_ && rw;

            if constexpr (requires { v.define_editor_fields(*this); })
                v.define_editor_fields(*this);
            else if constexpr (requires { define_editor_fields(*this, v); })
                define_editor_fields(*this, v);
            else
                static_assert(
                    detail::always_false_v<T>,
                    "ComponentInspector::field: type has no leaf overload, member define_editor_fields(), or ADL "
                    "define_editor_fields()");

            labelPrefix_ = savedPrefix;
            editableScope_ = savedScope;
        }

        [[nodiscard]] std::vector<InspectorField> Take() &&
        {
            return std::move(fields_);
        }
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
                     r.template get<T>(e).define_editor_fields(ci);
                     return std::move(ci).Take();
                 }});
        }

        [[nodiscard]] std::vector<InspectedComponent> Inspect(
            entt::registry& registry, entt::entity entity) const;
    };

    void RegisterDefaultInspectorComponents(InspectorRegistry& registry);
} // namespace sage::editor

// --- ADL-discoverable define_editor_fields overloads for raylib types --------
// Pattern mirrors engine/raylib-cereal.hpp: free functions in global namespace
// so unqualified `define_editor_fields(i, value)` finds them via ADL when the user
// composes a raylib type inside their component's define_editor_fields().

template <class Inspector>
void define_editor_fields(Inspector& i, BoundingBox& bb)
{
    i.field("Min", bb.min);
    i.field("Max", bb.max);
}
