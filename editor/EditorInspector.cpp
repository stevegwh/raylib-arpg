#include "EditorInspector.hpp"

#include "EditorComponents.hpp"
#include "engine/CollisionLayers.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/Spawner.hpp"
#include "engine/Light.hpp"

namespace sage::editor
{
    void ComponentInspector::Bool(std::string label, bool& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label), .kind = InspectorField::Kind::Bool, .data = &v, .editable = editable});
    }

    void ComponentInspector::Int(std::string label, int& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label), .kind = InspectorField::Kind::Int, .data = &v, .editable = editable});
    }

    void ComponentInspector::UInt(std::string label, unsigned int& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label), .kind = InspectorField::Kind::UInt, .data = &v, .editable = editable});
    }

    void ComponentInspector::UInt64(std::string label, std::uint64_t& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label),
             .kind = InspectorField::Kind::UInt64,
             .data = &v,
             .editable = editable});
    }

    void ComponentInspector::Float(std::string label, float& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label), .kind = InspectorField::Kind::Float, .data = &v, .editable = editable});
    }

    void ComponentInspector::String(std::string label, std::string& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label),
             .kind = InspectorField::Kind::String,
             .data = &v,
             .editable = editable});
    }

    void ComponentInspector::Vec2(std::string label, Vector2& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label), .kind = InspectorField::Kind::Vec2, .data = &v, .editable = editable});
    }

    void ComponentInspector::Vec3(std::string label, Vector3& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label), .kind = InspectorField::Kind::Vec3, .data = &v, .editable = editable});
    }

    void ComponentInspector::Color(std::string label, ::Color& v, const bool editable)
    {
        fields_.push_back(
            {.label = std::move(label), .kind = InspectorField::Kind::Color, .data = &v, .editable = editable});
    }

    void ComponentInspector::CollisionLayer(std::string label, sage::CollisionLayer& v, const bool editable)
    {
        InspectorField f{
            .label = std::move(label),
            .kind = InspectorField::Kind::Enum,
            .data = &v,
            .editable = editable};
        const auto& layers = GetCollisionLayers();
        f.enumOptions.reserve(layers.size());
        for (const auto& layer : layers) f.enumOptions.emplace_back(layer.layerName);
        f.getEnumIndex = [p = &v]() -> std::size_t {
            const auto& list = GetCollisionLayers();
            for (std::size_t i = 0; i < list.size(); ++i)
            {
                if (list[i].bit == p->bit) return i;
            }
            return 0;
        };
        f.setEnumIndex = [p = &v](const std::size_t idx) {
            const auto& list = GetCollisionLayers();
            if (idx < list.size()) *p = list[idx];
        };
        fields_.push_back(std::move(f));
    }

    std::vector<InspectedComponent> InspectorRegistry::Inspect(
        entt::registry& registry, const entt::entity entity) const
    {
        std::vector<InspectedComponent> result;
        for (const auto& entry : entries_)
        {
            if (!entry.has(registry, entity)) continue;
            result.push_back({entry.displayName, entry.describe(registry, entity)});
        }
        return result;
    }

    void RegisterDefaultInspectorComponents(InspectorRegistry& registry)
    {
        // Transform first so Position/Rotation/Scale sit at the top of the inspector.
        registry.Register<sgTransform>("Transform");
        registry.Register<PersistentEntityId>("Persistent Entity Id");
        registry.Register<EditorObjectDescriptor>("Editor Object");
        registry.Register<AssetReference>("Asset Reference");
        registry.Register<Renderable>("Renderable");
        registry.Register<Collideable>("Collideable");
        registry.Register<Light>("Light");
        registry.Register<Spawner>("Spawner");
    }
} // namespace sage::editor
