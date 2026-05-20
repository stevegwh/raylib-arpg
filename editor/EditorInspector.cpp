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
    void ComponentInspector::field(std::string label, sage::CollisionLayer& v, const bool ed)
    {
        EnumField e{.data = &v};
        const auto& layers = GetCollisionLayers();
        e.options.reserve(layers.size());
        for (const auto& layer : layers) e.options.emplace_back(layer.layerName);
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
        fields_.push_back(
            {.label = qualified(label), .editable = ed && editableScope_, .value = std::move(e)});
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
