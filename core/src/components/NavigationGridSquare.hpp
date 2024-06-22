//
// Created by Steve Wheeler on 25/02/2024.
//

#pragma once

#include <entt/entt.hpp>
#include "raylib.h"
#include "raymath.h"


namespace sage
{

struct NavigationGridSquare
{

    int pathfindingCost = 1;
    bool debugColor = false;
    Vector2 gridSquareIndex;
    Vector3 worldPosMin; // Top Left
    Vector3 worldPosMax; // Bottom Right
    Vector3 worldPosCentre;
    Vector3 debugBox;
	entt::entity occupant = entt::null;
    bool occupied = false;
    
    NavigationGridSquare(Vector2 _gridSquareIndex, 
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
