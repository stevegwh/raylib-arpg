//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <vector>
#include <raylib.h>

namespace sage
{
    class CollisionSystem
    {
        std::vector<BoundingBox> collideableBoundingBoxes;

    public:
        // Make const ref and store pointers to the actual bounding boxes?
        void AddCollideable(BoundingBox boundingBox);
        RayCollision CheckRayCollision(const Ray& ray);

    
    };
}

