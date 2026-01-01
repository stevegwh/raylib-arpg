//
// Created by steve on 30/12/2024.
//

#include "DoorSystem.hpp"

#include "EngineSystems.hpp"

#include "components/Collideable.hpp"
#include "components/DoorBehaviorComponent.hpp"
#include "components/sgTransform.hpp"
#include "systems/NavigationGridSystem.hpp"

namespace sage
{
    void DoorSystem::UnlockDoor(entt::entity entity) const
    {
        auto& door = registry->get<DoorBehaviorComponent>(entity);
        door.locked = false;
    }

    void DoorSystem::UnlockAndOpenDoor(entt::entity entity)
    {
        UnlockDoor(entity);
        OpenClickedDoor(entity);
    }

    void DoorSystem::OpenClickedDoor(entt::entity entity) const
    {
        auto& door = registry->get<DoorBehaviorComponent>(entity);
        if (door.locked) return;

        static const float closedRotation = 0.0f;

        auto& transform = registry->get<sgTransform>(entity);
        const auto& [rotx, roty, rotz] = transform.GetLocalRot();

        if (!door.open)
        {
            auto& col = registry->get<Collideable>(entity);
            col.collisionLayer = CollisionLayer::BACKGROUND;
            sys->navigationGridSystem->MarkSquareAreaOccupied(col.worldBoundingBox, false);
            float targetRotation = (transform.forward().z > 0) ? door.openYRotation : -door.openYRotation;
            transform.SetLocalRot(Vector3{rotx, targetRotation, rotz});
            door.open = true;
        }
        else
        {
            transform.SetLocalRot(Vector3{rotx, closedRotation, rotz});
            door.open = false;
            auto& col = registry->get<Collideable>(entity);
            col.collisionLayer = CollisionLayer::BUILDING;
            sys->navigationGridSystem->MarkSquareAreaOccupied(col.worldBoundingBox, true);
        }
    }

    DoorSystem::DoorSystem(entt::registry* _registry, sage::EngineSystems* _sys) : registry(_registry), sys(_sys)
    {
    }
} // namespace sage