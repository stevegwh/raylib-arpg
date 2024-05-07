//
// Created by Steve Wheeler on 25/02/2024.
//

#pragma once

#include "Collideable.hpp"

#include "raylib.h"
#include "raymath.h"

#include <vector>

namespace sage
{
    struct NavigationGridSquare
    {
        bool debugColor = false;
        const Vector3 worldPosMin; // Top Left
        const Vector3 worldPosMax; // Bottom Right
        const Vector3 worldPosCentre;
        const Vector3 debugBox;
        std::vector<CollisionInfo> collisionsWithSquare;
        bool occupied = false;
        Vector2 gridSquareIndex;
        
        explicit NavigationGridSquare(Vector2 _gridSquareIndex, 
                                      Vector3 _worldPosMin, 
                                      Vector3 _worldPosMax, 
                                      Vector3 _worldPosCentre) : 
        gridSquareIndex(_gridSquareIndex), 
        worldPosMin(_worldPosMin), 
        worldPosMax(_worldPosMax), 
        worldPosCentre(_worldPosCentre),
        debugBox({ fabsf(worldPosMax.x - worldPosMin.x), 
                   0.1f, 
                   fabsf(worldPosMax.z - worldPosMin.z) })
        {}
        
    };
}