//
// Created by Steve Wheeler on 18/02/2024.
//

#include <algorithm>

#include "CollisionSystem.hpp"

bool compareCollisionDistances(const sage::CollisionInfo& a, const sage::CollisionInfo& b)
{
    return a.rayCollision.distance < b.rayCollision.distance;
}


namespace sage
{

    std::vector<CollisionInfo> CollisionSystem::CheckRayCollision(const Ray& ray) const
    {
        std::vector<CollisionInfo> collisions;

        for (const auto& c : components)
        {
            auto col = GetRayCollisionBox(ray, c.second->worldBoundingBox);
            if (col.hit) 
            {

                CollisionInfo info = {
                    .collidedEntityId= c.second->entityId,
                    .rayCollision = col
                };
                collisions.push_back(info);
            }
        }

        std::sort(collisions.begin(), collisions.end(), compareCollisionDistances);

        return collisions;
    }

    void CollisionSystem::BoundingBoxDraw(EntityID entityId, Color color) const
    {
        auto bb = GetComponent(entityId).worldBoundingBox;
        DrawBoundingBox(bb, color);
    }

    void CollisionSystem::SetBoundingBox(EntityID entityId, BoundingBox bb)
    {
        components.at(entityId)->worldBoundingBox = bb;
    }

}