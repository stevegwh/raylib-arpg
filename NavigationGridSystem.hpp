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
    void init();
    std::vector<std::vector<NavigationGridSquare*>> gridSquares;
    float spacing;
    int slices;
public:
    void PopulateGrid();
    Vector2 WorldToGridSpace(Vector3 worldPos);
    std::vector<Vector3> Pathfind(const Vector3& startPos, const Vector3& finishPos,
                                  EntityID startGridId, EntityID finishGridId);
    const std::vector<std::vector<NavigationGridSquare*>>& GetGridSquares();

    NavigationGridSystem(int _slices, float _spacing)
    : slices(_slices), spacing(_spacing)
    {
        init();
    }
};

} // sage
