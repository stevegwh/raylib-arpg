//
// Created by Steve Wheeler on 16/06/2024.
//

#pragma once

#include "NavigationGridSquare.hpp"
#include "raylib.h"
#include <entt/entt.hpp>

#include <deque>
#include <vector>

namespace sage
{
    struct MoveableActor
    {
        float movementSpeed = 0.35f;

        // Collision detection
        entt::entity lastHitActor = entt::null;
        Vector3 hitActorLastPos{};
        // ----

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

        // TODO: Not a huge fan of how much there is to do here.
        entt::entity targetActor = entt::null;
        double lastTargetPosCheck{};
        float lastTargetPosCheckThreshold = 1.0;
        int onTargetPosUpdateHookId;
        entt::sigh<void(entt::entity, entt::entity)> onTargetPosUpdate{}; // Self, target
    };
} // namespace sage
