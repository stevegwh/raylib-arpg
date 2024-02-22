//
// Created by Steve Wheeler on 18/02/2024.
//

#include "CollisionSystem.hpp"


namespace sage
{

    CollisionInfo CollisionSystem::CheckRayCollision(const Ray& ray) const
    {
        for (const auto& c : components)
        {
            auto col = GetRayCollisionBox(ray, c.second->boundingBox);
            if (col.hit) 
            {
                //c.OnCollisionHit.callback();

                CollisionInfo info = {
                    .collidedEntityId= c.second->entityId,
                    .rayCollision = col
                };
                
                // TODO: add to an array and return all collisions
                return info;
            }
        }

        return {0};
    }

    void CollisionSystem::BoundingBoxDraw(EntityID entityId, Color color) const
    {
        auto bb = GetComponent(entityId).boundingBox;
        DrawBoundingBox(bb, color);
    }

}