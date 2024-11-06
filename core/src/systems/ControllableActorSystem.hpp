//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    class GameData;

    class ControllableActorSystem : public BaseSystem
    {
        GameData* gameData;
        entt::entity selectedActorId{};
        void onTargetUpdate(entt::entity target);

      public:
        void MoveToLocation(entt::entity id);
        void PathfindToLocation(entt::entity id, Vector3 location) const;
        void CancelMovement(entt::entity entity);
        void SetSelectedActor(entt::entity id);
        entt::entity GetSelectedActor();

        void PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol);
        [[nodiscard]] bool ReachedDestination(entt::entity entity) const;

        entt::sigh<void(entt::entity)> onSelectedActorChange;
        void Update() const;
        ControllableActorSystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage
