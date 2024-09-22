//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Timer.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
    class Cursor;

    class ControllableActor
    {
        entt::entity self;

      public:
        entt::sigh<void(entt::entity, entt::entity)> onEnemyLeftClick{};  // Self, Clicked enemy
        entt::sigh<void(entt::entity, entt::entity)> onEnemyRightClick{}; // Self, Clicked enemy
        entt::sigh<void(entt::entity, entt::entity)> onFloorClick{};      // Self, object clicked (can discard)

        // TODO: Remove this or move it from controllable actor
        // Timer to check if the target has moved.
        Timer checkTargetPosTimer{};
        // An actor that is the target for pathfinding etc.
        entt::entity targetActor = entt::null;

        // The max range the actor can pathfind at one time.
        int pathfindingBounds = 100;
        Vector3 targetActorPos{};
        ControllableActor(entt::entity _self, Cursor* _cursor);
    };
} // namespace sage
