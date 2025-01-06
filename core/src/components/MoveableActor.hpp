//
// Created by Steve Wheeler on 16/06/2024.
//

#pragma once

#include "NavigationGridSquare.hpp"

#include "raylib.h"
#include <entt/entt.hpp>
#include <Event.hpp>

#include <deque>
#include <memory>
#include <optional>
#include <vector>

namespace sage
{
    class FollowTarget
    {
        entt::registry* registry;
        entt::entity self{}; // The actor following
        std::unique_ptr<Connection> onTargetDestinationReachedCnx{};
        std::unique_ptr<Connection> onTargetMovementCancelledCnx{};
        std::unique_ptr<Connection> onTargetPathChangedCnx{};

        Vector3 targetPrevPos{};
        double timeStarted{};
        float timerThreshold = 0.25;

      public:
        const entt::entity targetActor = entt::null;
        std::unique_ptr<Event<entt::entity, entt::entity>> onTargetDestinationReached{}; // Self, target
        std::unique_ptr<Event<entt::entity, entt::entity>> onTargetMovementCancelled{};  // Self, target
        std::unique_ptr<Event<entt::entity, entt::entity>> onTargetPathChanged{};        // Self, target

        ~FollowTarget();
        FollowTarget(entt::registry* _registry, entt::entity _self, entt::entity _targetActor);
    };

    struct MoveableActorCollision
    {
        entt::entity hitEntityId = entt::null;
        Vector3 hitLastPos{};
    };

    struct MoveableActor
    {
        float movementSpeed = 0.35f;
        // The max range the actor can pathfind at one time.
        int pathfindingBounds = 50;
        // std::optional<MoveableActorCollision> moveableActorCollision;
        entt::entity hitEntityId = entt::null;
        Vector3 hitLastPos{};
        std::optional<FollowTarget> followTarget;
        std::deque<Vector3> path{};

        std::unique_ptr<Event<entt::entity>> onStartMovement{};
        std::unique_ptr<Event<entt::entity>> onDestinationReached{};
        std::unique_ptr<Event<entt::entity, Vector3>> onDestinationUnreachable{}; // self, original dest
        std::unique_ptr<Event<entt::entity>> onPathChanged{}; // Was previously moving, now moving somewhere else
        std::unique_ptr<Event<entt::entity>> onMovementCancel{}; // Was previously moving, now cancelled

        [[nodiscard]] bool IsMoving() const
        {
            return !path.empty();
        }

        [[nodiscard]] Vector3 GetDestination() const
        {
            assert(IsMoving()); // Check this independently before calling this function.
            return path.back();
        }

        std::vector<GridSquare> debugRay;

        MoveableActor();
    };
} // namespace sage
