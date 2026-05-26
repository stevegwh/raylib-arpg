#include "EditorInspector.hpp"

#include "EditorComponents.hpp"
#include "engine/CollisionLayers.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/Spawner.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "engine/Light.hpp"

#include <cassert>

namespace sage::editor
{
    void ComponentInspector::field(const std::string& label, sage::CollisionLayer& v, const bool ed)
    {
        EnumField e{.data = &v};
        const auto& layers = GetCollisionLayers();
        e.options.reserve(layers.size());
        for (const auto& layer : layers)
            e.options.emplace_back(layer.layerName);
        e.getIndex = [p = &v]() -> std::size_t {
            const auto& list = GetCollisionLayers();
            for (std::size_t i = 0; i < list.size(); ++i)
            {
                if (list[i].bit == p->bit) return i;
            }
            return 0;
        };
        e.setIndex = [p = &v](const std::size_t idx) {
            const auto& list = GetCollisionLayers();
            if (idx < list.size()) *p = list[idx];
        };
        fields_.push_back({.label = qualified(label), .editable = ed && editableScope_, .value = std::move(e)});
    }

    void InspectorRegistry::RegisterTransform(std::string displayName)
    {
        entries_.push_back(
            {std::move(displayName),
             [](const entt::registry& r, const entt::entity e) {
                 return r.valid(e) && r.any_of<sgTransform>(e);
             },
             [](entt::registry& r, const entt::entity e, TransformSystem*) {
                 ComponentInspector ci;
                 auto& transform = r.get<sgTransform>(e);
                 auto* reg = &r;
                 ci.field(
                     "Position",
                     const_cast<Vector3&>(transform.GetLocalPos()),
                     [reg, e](const Vector3& position) {
                         reg->get<sgTransform>(e).position.local = position;
                     });
                 ci.field(
                     "Rotation",
                     const_cast<Vector3&>(transform.GetLocalRot()),
                     [reg, e](const Vector3& rotation) {
                         reg->get<sgTransform>(e).rotation.local = rotation;
                     });
                 ci.field(
                     "Scale",
                     const_cast<Vector3&>(transform.GetLocalScale()),
                     [reg, e](const Vector3& scale) {
                         reg->get<sgTransform>(e).scale.local = scale;
                     });
                 return std::move(ci).Take();
             }});
    }

    std::vector<InspectedComponent> InspectorRegistry::Inspect(
        entt::registry& registry, const entt::entity entity, TransformSystem* transformSystem) const
    {
        std::vector<InspectedComponent> result;
        for (const auto& entry : entries_)
        {
            if (!entry.has(registry, entity)) continue;
            result.push_back({entry.displayName, entry.describe(registry, entity, transformSystem)});
        }
        return result;
    }

    void RegisterDefaultInspectorComponents(InspectorRegistry& registry)
    {
        // Transform first so Position/Rotation/Scale sit at the top of the inspector.
        registry.RegisterTransform("Transform");
        registry.Register<PersistentEntityId>("Persistent Entity Id");
        registry.Register<EditorObjectDescriptor>("Editor Object");
        registry.Register<AssetReference>("Asset Reference");
        registry.Register<Renderable>("Renderable");
        registry.Register<Collideable>("Collideable");
        registry.Register<Light>("Light");
        registry.Register<Spawner>("Spawner");
    }
} // namespace sage::editor
