//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Timer.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
    struct ControllableActor
    {
        // The time between checks for the target position.
        Timer checkTargetPosTimer{};
        // The max range the actor can pathfind at one time.
        int pathfindingBounds = 25;
        // An actor that is the target for pathfinding etc.
        entt::entity targetActor = entt::null;
        Vector3 targetActorPos{};
    };
} // namespace sage
