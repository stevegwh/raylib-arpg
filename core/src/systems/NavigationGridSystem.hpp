//
// Created by Steve Wheeler on 25/02/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "../components/NavigationGridSquare.hpp"
#include "CollisionSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{

class NavigationGridSystem : public BaseSystem<NavigationGridSquare>
{
    std::vector<std::vector<NavigationGridSquare*>> gridSquares;
public:
    float spacing{};
    int slices{};

    NavigationGridSystem(entt::registry* _registry);
    void Init(int _slices, float _spacing);
    void PopulateGrid();
    bool WorldToGridSpace(Vector3 worldPos, Vector2& out);
    std::vector<Vector3> Pathfind(const Vector3& startPos, const Vector3& finishPos);
    const std::vector<std::vector<NavigationGridSquare*>>& GetGridSquares();
};

} // sage
