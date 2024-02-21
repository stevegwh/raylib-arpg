//
// Created by Steve Wheeler on 18/02/2024.
//

#include "CollisionSystem.hpp"


namespace sage
{
    void CollisionSystem::AddCollideable(Collideable& collideable)
    {
        collideables[collideable.entityId] = std::make_unique<Collideable>(collideable);
    }

    const Collideable& CollisionSystem::GetComponent(EntityID entityId)
    {
        return *collideables.at(entityId);
    }

    CollisionInfo CollisionSystem::CheckRayCollision(const Ray& ray)
    {
        for (const auto& c : collideables)
        {
            auto col = GetRayCollisionBox(ray, c.second->boundingBox);
            if (col.hit) 
            {
                //c.OnCollisionHit.callback();

                CollisionInfo info = {
                    .collidedObject= c.second->entityId,
                    .rayCollision = col
                };
                
                // TODO: add to an array and return all collisions
                return info;
            }
        }

        return {0};
    }

}