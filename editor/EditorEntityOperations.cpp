#include "EditorEntityOperations.hpp"

#include "engine/EngineSystems.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "engine/systems/TransformSystem.hpp"

#include <vector>

namespace sage::editor
{
    EditorEntityOperations::EditorEntityOperations(EngineSystems* _sys) : sys(_sys)
    {
    }

    void EditorEntityOperations::DeleteEntityAndChildren(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity)) return;

        std::vector<entt::entity> children;
        if (sys->registry->any_of<sgTransform>(entity))
        {
            children = sys->registry->get<sgTransform>(entity).GetChildren();
        }

        for (const auto child : children)
        {
            DeleteEntityAndChildren(child);
        }

        if (sys->registry->valid(entity) && sys->registry->any_of<sgTransform>(entity))
        {
            sys->transformSystem->SetParent(entity, entt::null);
        }

        releaseNavigationOccupation(entity);

        if (sys->registry->valid(entity))
        {
            sys->registry->destroy(entity);
        }
    }

    void EditorEntityOperations::releaseNavigationOccupation(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<Collideable>(entity)) return;

        const auto& collideable = sys->registry->get<Collideable>(entity);
        if (collideable.blocksNavigation)
        {
            sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false, entity);
        }
    }
} // namespace sage::editor
