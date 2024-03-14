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
        const Vector3 worldPosMin; // Top Left
        const Vector3  worldPosMax; // Bottom Right
        const Vector3 worldPosCentre;
        std::vector<CollisionInfo> collisionsWithSquare;
        bool occupied = false;
        int gridSquareIndex;
        
        explicit NavigationGridSquare(EntityID id, int _gridSquareIndex, Vector3 _worldPosMin, Vector3 _worldPosMax, Vector3 _worldPosCentre) 
        : gridSquareIndex(_gridSquareIndex), worldPosMin(_worldPosMin), worldPosMax(_worldPosMax), worldPosCentre(_worldPosCentre),
        Component(id) {}
        
    };
}