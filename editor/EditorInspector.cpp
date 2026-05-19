#include "EditorInspector.hpp"

#include "EditorComponents.hpp"
#include "engine/Light.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/Spawner.hpp"
#include "engine/components/sgTransform.hpp"

namespace sage::editor
{
    std::vector<InspectedComponent> ComponentInspectorRegistry::InspectEntity(
        entt::registry& registry,
        const entt::entity entity) const
    {
        std::vector<InspectedComponent> inspected;
        for (const auto& entry : entries)
        {
            if (!entry.hasComponent || !entry.inspect || !entry.hasComponent(registry, entity)) continue;

            auto component = entry.inspect(registry, entity);
            component.displayName = entry.displayName;
            inspected.push_back(std::move(component));
        }
        return inspected;
    }

    void RegisterDefaultInspectorComponents(ComponentInspectorRegistry& registry)
    {
        registry.Register<PersistentEntityId>("Persistent Entity Id");
        registry.Register<EditorObjectDescriptor>("Editor Object");
        registry.Register<AssetReference>("Asset Reference");

        registry.Register<sgTransform>("Transform");

        registry.Register<Renderable>(
            "Renderable",
            [](Renderable& renderable, Inspector& inspector) {
                inspector.field("Active", renderable.active);
                inspector.field("Serializable", renderable.serializable);
                inspector.readOnlyField("Name", renderable.GetName());
                inspector.readOnlyField("Vanity Name", renderable.GetVanityName());
                if (const auto* model = renderable.GetModel(); model != nullptr)
                {
                    inspector.readOnlyField("Model Key", model->GetKey());
                }
            });

        registry.Register<Collideable>(
            "Collideable",
            [](Collideable& collideable, Inspector& inspector) {
                inspector.field("Active", collideable.active);
                inspector.field("Debug Draw", collideable.debugDraw);
                inspector.field("Blocks Navigation", collideable.blocksNavigation);
                inspector.field("Collision Layer Bit", collideable.collisionLayer.bit);
                inspector.readOnlyField("Local Bounds Min", collideable.localBoundingBox.min);
                inspector.readOnlyField("Local Bounds Max", collideable.localBoundingBox.max);
                inspector.readOnlyField("World Bounds Min", collideable.worldBoundingBox.min);
                inspector.readOnlyField("World Bounds Max", collideable.worldBoundingBox.max);
            });

        registry.Register<Light>(
            "Light",
            [](Light& light, Inspector& inspector) {
                inspector.field("Enabled", light.enabled);
                inspector.field("Type", light.type);
                inspector.field("Position", light.position);
                inspector.field("Target", light.target);
                inspector.field("Color", light.color);
                inspector.field("Brightness", light.brightness).range(0.0, 20.0).step(0.1);
                inspector.field("Constant", light.constant).min(0.0).step(0.1);
                inspector.field("Linear", light.linear).min(0.0).step(0.1);
                inspector.field("Quadratic", light.quadratic).min(0.0).step(0.1);
            });

        registry.Register<Spawner>(
            "Spawner",
            [](Spawner& spawner, Inspector& inspector) {
                inspector.field("Name", spawner.name);
                inspector.field("Type", spawner.type);
                inspector.field("Position", spawner.pos);
                inspector.field("Rotation", spawner.rot);
            });
    }
} // namespace sage::editor
