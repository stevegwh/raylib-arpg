//
// Created by Steve Wheeler on 25/02/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "NavigationGridSquare.hpp"
#include "CollisionSystem.hpp"


namespace sage
{

class NavigationGridSystem : public BaseSystem<NavigationGridSquare>
{
    void init(int slices, float spacing, CollisionSystem& collisionSystem);
public:
    std::vector<NavigationGridSquare> unoccupiedSquares;
    void FindOccupiedGridSquares();

    NavigationGridSystem(int slices, float spacing, CollisionSystem& collisionSystem)
    {
        init(slices, spacing, collisionSystem);
    }
};

} // sage