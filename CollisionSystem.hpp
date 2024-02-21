//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <vector>
#include <raylib.h>
#include "Collideable.hpp"

namespace sage
{
    class CollisionSystem
    {
        std::vector<Collideable> collideables;

    public:
        // Make const ref and store pointers to the actual bounding boxes?
        void AddCollideable(Collideable collideable);
        CollisionInfo CheckRayCollision(const Ray& ray);
    };
}

