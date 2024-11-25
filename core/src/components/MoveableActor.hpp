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
        entt::registry* registry;
        entt::entity self{};
        EntityReflectionSignalRouter* reflectionSignalRouter;
        int onTargetPosUpdateHookId{};

        Vector3 targetPrevPos{};
        double lastCheckTime{};
        float timerThreshold = 0.25;
        void checkTargetPos(); // Called from ActorMovementSystem
      public:
        const entt::entity targetActor = entt::null;
        entt::sigh<void(entt::entity, entt::entity)> onTargetPosUpdate{}; // Self, target

        ~FollowTarget();
        explicit FollowTarget(
            entt::registry* _registry,
            EntityReflectionSignalRouter* _reflectionSignalRouter,
            entt::entity _self,
            entt::entity _targetActor);

        friend class ActorMovementSystem;
    };

    struct MoveableActorCollision
    {
        entt::entity hitEntityId = entt::null;
        Vector3 hitLastPos{};
    };

    enum class DestinationUnreachableBehaviour
    {
        CANCEL,
        WAIT // Retry
    };

    struct MoveableActor
    {
        DestinationUnreachableBehaviour destUnreachableBehaviour;
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
        // entt::sigh<void(entt::entity)> onFinishMovement{};
        entt::sigh<void(entt::entity)> onDestinationReached{};
        entt::sigh<void(entt::entity)> onPathChanged{};    // Was previously moving, now moving somewhere else
        entt::sigh<void(entt::entity)> onMovementCancel{}; // Was previously moving, now cancelled
    };
} // namespace sage
