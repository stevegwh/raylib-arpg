//
// Created by Steve Wheeler on 25/02/2024.
//

#pragma once

#include "Component.hpp"
#include "Collideable.hpp"

#include "raylib.h"
#include "raymath.h"

#include <vector>

namespace sage
{
    struct NavigationGridSquare : public Component
    {
        Vector3 squareCentre{};
        std::vector<CollisionInfo> collisionsWithSquare;
        bool occupied = false;
        const int gridSquareIndex;
        
        explicit NavigationGridSquare(EntityID id, int _gridSquareIndex) : gridSquareIndex(_gridSquareIndex), Component(id) {}
        
    };
}