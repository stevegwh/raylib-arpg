//
// Created by Steve Wheeler on 04/06/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{

struct Combatable
{
    bool inCombat = false;
    entt::entity target;
    float autoAttackTick = 0;
    float autoAttackTickThreshold = 1;
};

} // sage
