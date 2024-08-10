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
    struct MoveableActor
    {
        entt::entity lastHitActor = entt::null;
        Vector3 hitActorLastPos{};
        std::deque<Vector3> path{};
        bool isMoving() const
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
