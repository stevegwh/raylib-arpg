//
// Created by Steve Wheeler on 21/02/2024.
//

#include "TransformSystem.hpp"

#include "Game.hpp"

namespace sage
{
    void TransformSystem::SetComponent(EntityID entityId, Transform newTransform)
    {
        components.at(entityId)->position = newTransform.position;
    }

    void findExistingTransform(EntityID entityId)
    {

    }

    void TransformSystem::MoveToLocation(EntityID entityId, Vector3 location)
    {
        // Prune existing move commands
        // TODO: improve
        for (auto it = moveTowardsTransforms.begin(); it != moveTowardsTransforms.end();)
        {
            if ((*it)->entityId == entityId)
            {
                moveTowardsTransforms.erase(it);
                continue;
            }
            ++it;
        }

        auto transform = components.at(entityId).get();
        transform->target = location;
        transform->direction = Vector3Normalize(Vector3Subtract(transform->target, transform->position));
        moveTowardsTransforms.push_back(transform);
    }
    
    void TransformSystem::Update()
    {
        for (auto it = moveTowardsTransforms.begin(); it != moveTowardsTransforms.end();) 
        {
            const auto& transform = *it;
            
            if (Vector3Distance(transform->target, transform->position) < 0.5f)
            {
                it = moveTowardsTransforms.erase(it);
                continue;
            }
            
            transform->position.x = transform->position.x + transform->direction.x * 0.5f;
            //transform->position.x = dy * 0.5f;
            transform->position.z = transform->position.z + transform->direction.z * 0.5f;

            // TODO: This is not a scalable solution. (Need to figure out how to update components that depend on this one)
            if (Game::GetInstance().collisionSystem->FindEntity(transform->entityId))
            {
                Game::GetInstance().collisionSystem->UpdateWorldBoundingBox(transform->entityId, transform->position);
            }

            ++it;
        }

    }
    
    
}
