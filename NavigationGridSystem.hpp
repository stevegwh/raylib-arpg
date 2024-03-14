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
    void init(CollisionSystem& collisionSystem);
    std::vector<NavigationGridSquare*> gridSquares;
    float spacing;
    int slices;
public:
    void PopulateGrid();
    int WorldToGridSpace(Vector3 worldPos);
    std::vector<Vector3> Pathfind(const Vector3& startPos, const Vector3& finishPos,
                                  EntityID startGridId, EntityID finishGridId);
    const std::vector<NavigationGridSquare*>& GetGridSquares();

    NavigationGridSystem(int _slices, float _spacing, CollisionSystem& collisionSystem)
    : slices(_slices), spacing(_spacing)
    {
        init(collisionSystem);
    }
};

} // sage
