//
// Created by Steve Wheeler on 16/06/2024.
//

#pragma once

#include "NavigationGridSquare.hpp"
#include "raylib.h"

#include <entt/entt.hpp>

#include <deque>
#include <optional>
#include <vector>

namespace sage
{
    class EntityReflectionSignalRouter;

    class FollowTarget
    {
        EntityReflectionSignalRouter* reflectionSignalRouter;
        int onTargetPosUpdateHookId{};

        double lastCheckTime{};
        float timerThreshold = 1.0;

      public:
        const entt::entity targetActor = entt::null;
        entt::sigh<void(entt::entity, entt::entity)> onTargetPosUpdate{}; // Self, target

        [[nodiscard]] bool ShouldCheckTargetPos();

        ~FollowTarget();
        explicit FollowTarget(
            entt::registry* _registry,
            EntityReflectionSignalRouter* _reflectionSignalRouter,
            entt::entity self,
            entt::entity _targetActor);
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
        [[nodiscard]] bool isMoving() const
        {
            return !path.empty();
        }
        std::vector<GridSquare> debugRay;

        entt::sigh<void(entt::entity)> onStartMovement{};
        entt::sigh<void(entt::entity)> onFinishMovement{};
        entt::sigh<void(entt::entity)> onDestinationReached{};
        entt::sigh<void(entt::entity)> onMovementCancel{};
    };
} // namespace sage
