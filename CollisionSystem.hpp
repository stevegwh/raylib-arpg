//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <map>
#include <raylib.h>
#include "Collideable.hpp"

namespace sage
{
    class CollisionSystem
    {
        std::map<EntityID, std::unique_ptr<Collideable>> collideables;

    public:
        // Make const ref and store pointers to the actual bounding boxes?
        void AddCollideable(Collideable& collideable);
        CollisionInfo CheckRayCollision(const Ray& ray);
        const Collideable& GetComponent(EntityID entityId);
    };
}

