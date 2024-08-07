//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "components/ControllableActor.hpp"

#include "BaseSystem.hpp"

#include "entt/entt.hpp"
#include "raylib.h"
#include "raymath.h"

#include <vector>

namespace sage
{
    class GameData;

    class ControllableActorSystem : public BaseSystem
    {
        GameData* gameData;
        entt::entity controlledActorId{};
        // TODO: Right now this system just controls one unit at a time. What if we wanted
        // more?
        void onFloorClick(entt::entity entity);
        void onEnemyClick(entt::entity clickedEntity);
        void onTargetUpdate(entt::entity target);

      public:
        void MoveToLocation(entt::entity id);
        void PathfindToLocation(entt::entity id, Vector3 location);
        void CancelMovement(entt::entity entity);
        void SetControlledActor(entt::entity id);
        entt::entity GetControlledActor();

        void PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol);
        bool ReachedDestination(entt::entity entity) const;

        entt::sigh<void(entt::entity)> onControlledActorChange;
        void Enable();
        void Disable();
        void Update() const;
        ControllableActorSystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage
