//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <map>
#include <raylib.h>
#include "Collideable.hpp"
#include "BaseSystem.hpp"

namespace sage
{
class CollisionSystem : public BaseSystem<Collideable>
{
public:
    CollisionInfo CheckRayCollision(const Ray& ray);
};
}

