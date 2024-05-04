//
// Created by Steve Wheeler on 18/02/2024.
//

#include <algorithm>

#include "CollisionSystem.hpp"

#include "../Serializer.hpp"
#include "../GameManager.hpp"

bool compareRayCollisionDistances(const sage::CollisionInfo& a, const sage::CollisionInfo& b)
{
    return a.rayCollision.distance < b.rayCollision.distance;
}


namespace sage
{

//    void CollisionSystem::TransformUpdateSubscribe(entt::entity entity) // TODO: I don't like this solution
//    {
//        eventManager->Subscribe([this, entity] { onTransformUpdate(entity); },
//                                *registry->get<Transform>(entity).OnPositionUpdate);
//    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(const Ray& ray) const
    {
        std::vector<CollisionInfo> collisions;
        
        auto view = registry->view<Collideable>();

        view.each([&collisions, ray](auto entity, const auto& c)
        {
            if (c.collisionLayer != NAVIGATION) // TODO: Need to define a collision matrix
            {
                auto col = GetRayCollisionBox(ray, c.worldBoundingBox);
                if (col.hit)
                {

                    CollisionInfo info = {
                        .collidedEntityId = entity,
                        .collidedBB = c.worldBoundingBox,
                        .rayCollision = col
                    };
                    collisions.push_back(info);
                }
            }
        });

        std::sort(collisions.begin(), collisions.end(), compareRayCollisionDistances);

        return collisions;
    }

    void CollisionSystem::BoundingBoxDraw(entt::entity entityId, Color color) const
    {
        DrawBoundingBox(registry->get<Collideable>(entityId).worldBoundingBox, color);
    }

/**
     * Calculates worldBoundingBox by multiplying localBoundingBox with the passed transform matrix
     * @param entityId The id of the entity
     * @param mat The transform matrix for the local bounding box
     */
    void CollisionSystem::UpdateWorldBoundingBox(entt::entity entityId, Matrix mat) // TODO: I don't like the name
    {
        registry->patch<Collideable>(entityId, [mat](auto& col) {
            auto bb = col.localBoundingBox;
            bb.min = Vector3Transform(bb.min, mat);
            bb.max = Vector3Transform(bb.max, mat);
            col.worldBoundingBox = bb;
        });
    }

//    void CollisionSystem::onTransformUpdate(entt::entity entityId)
//    {
//        Matrix mat = ECS->transformSystem->GetMatrixNoRot(entityId);
//        UpdateWorldBoundingBox(entityId, mat);
//    }    
    
    bool CollisionSystem::CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2) 
    {
        return CheckCollisionBoxes(col1, col2);
    }
    
    bool CollisionSystem::checkCollisionMatrix(const CollisionLayer& layer1, const CollisionLayer& layer2)
    {
        const auto& layerMatrix = collisionMatrix.at(layer1);
        return std::find(layerMatrix.begin(), layerMatrix.end(), layer2) != layerMatrix.end();
    }

    bool CollisionSystem::GetFirstCollision(entt::entity entity)
    {
        const auto& targetCol = registry->get<Collideable>(entity);

        auto view = registry->view<Collideable>();

        for (const auto& ent: view) 
        {
            const auto& c = view.get<Collideable>(ent);
            if (c.collisionLayer != BUILDING) continue; // TODO: Wanted to query the collision matrix but is far too slow
            bool colHit = CheckBoxCollision(targetCol.worldBoundingBox, c.worldBoundingBox);
            if (colHit) return true;
        }
        return false;
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisions(entt::entity entity)
    {
        std::vector<CollisionInfo> collisions;
        
        const Collideable& targetCol = registry->get<Collideable>(entity);
        auto view = registry->view<Collideable>();

        for (const auto& ent: view)
        {
            const auto& c = view.get<Collideable>(ent);
            if (ent == entity) continue;
            if (!checkCollisionMatrix(targetCol.collisionLayer, c.collisionLayer)) continue;
            
            bool colHit = CheckBoxCollision(targetCol.worldBoundingBox, c.worldBoundingBox);
            if (colHit)
            {
                CollisionInfo info = {
                    .collidedEntityId = ent,
                    .collidedBB = c.worldBoundingBox
                };
                
                collisions.push_back(info);
            }
        }

        return collisions;
    }

    void CollisionSystem::DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data)
    {
        
    }
    
    CollisionSystem::CollisionSystem(entt::registry *_registry) :
        BaseSystem<Collideable>(_registry)
    {}

}