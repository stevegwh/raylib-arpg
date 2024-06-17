//
// Created by Steve Wheeler on 16/06/2024.
//

#pragma once

#include "raylib.h"

#include <deque>

namespace sage
{

struct MoveableActor
{
    std::deque<Vector3> globalPath{};
};

} // sage
