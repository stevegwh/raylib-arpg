//
// Created by Steve Wheeler on 04/06/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{

struct Combatable
{
    entt::entity target;
    float currentTick;
    float maxTick;
};

} // sage
