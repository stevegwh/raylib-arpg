//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <map>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include "Collideable.hpp"
#include "BaseSystem.hpp"

namespace sage
{
class CollisionSystem : public BaseSystem<Collideable>
{
public:
    [[nodiscard]] std::vector<CollisionInfo> CheckRayCollision(const Ray& ray) const;
    void BoundingBoxDraw(EntityID entityId, Color color = LIME) const;
};
}

