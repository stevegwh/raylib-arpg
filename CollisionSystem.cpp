//
// Created by Steve Wheeler on 18/02/2024.
//

#include "CollisionSystem.hpp"


namespace sage
{
    void CollisionSystem::AddCollideable(BoundingBox boundingBox)
    {
        collideableBoundingBoxes.push_back(boundingBox);
    }

    RayCollision CollisionSystem::CheckRayCollision(const Ray& ray)
    {
        for (const auto& bb : collideableBoundingBoxes)
        {
            auto col = GetRayCollisionBox(ray, bb);
            if (col.hit)
            {
                // TODO: add to an array and return all collisions
                return col;
            }
        }

        RayCollision col;
        col.hit = false;
        return col;
    }

}