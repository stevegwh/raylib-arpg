//
// Created by Steve Wheeler on 18/02/2024.
//

#include <algorithm>

#include "CollisionSystem.hpp"

#include "Serializer.hpp"
#include "GameManager.hpp"

bool compareRayCollisionDistances(const sage::CollisionInfo& a, const sage::CollisionInfo& b)
{
    return a.rayCollision.distance < b.rayCollision.distance;
}


namespace sage
{

    std::vector<CollisionInfo> CollisionSystem::GetCollisionsWithRay(const Ray& ray) const
    {
        std::vector<CollisionInfo> collisions;

        for (const auto& c : components)
        {
            if (c.second->collisionLayer == NAVIGATION) continue; // TODO: Need to define a collision matrix
            auto col = GetRayCollisionBox(ray, c.second->worldBoundingBox);
            if (col.hit) 
            {

                CollisionInfo info = {
                    .collidedEntityId= c.second->entityId,
                    .collidedBB = c.second->worldBoundingBox,
                    .rayCollision = col
                };
                collisions.push_back(info);
            }
        }

        std::sort(collisions.begin(), collisions.end(), compareRayCollisionDistances);

        return collisions;
    }

    void CollisionSystem::BoundingBoxDraw(EntityID entityId, Color color) const
    {
        auto bb = components.at(entityId)->worldBoundingBox;
        DrawBoundingBox(bb, color);
    }

/**
     * Calculates worldBoundingBox by multiplying localBoundingBox with the passed transform matrix
     * @param entityId The id of the entity
     * @param mat The transform matrix for the local bounding box
     */
    void CollisionSystem::UpdateWorldBoundingBox(EntityID entityId, Matrix mat)
    {
        auto bb = GetComponent(entityId)->localBoundingBox;
        bb.min = Vector3Transform(bb.min, mat);
        bb.max = Vector3Transform(bb.max, mat);
        components.at(entityId)->worldBoundingBox = bb;
    }

    void CollisionSystem::onTransformUpdate(EntityID entityId)
    {
        Matrix mat = ECS->transformSystem->GetMatrixNoRot(entityId);
        UpdateWorldBoundingBox(entityId, mat);
    }    
    
    bool CollisionSystem::CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2) 
    {
        return CheckCollisionBoxes(col1, col2);
    }
    
    bool CollisionSystem::checkCollisionMatrix(const CollisionLayer& layer1, const CollisionLayer& layer2)
    {
        const auto& layerMatrix = collisionMatrix.at(layer1);
        return std::find(layerMatrix.begin(), layerMatrix.end(), layer2) != layerMatrix.end();
    }

    bool CollisionSystem::GetFirstCollision(EntityID entity)
    {
        const Collideable& targetCol = *components.at(entity);
        for (const auto& c : components)
        {
            if (c.second->collisionLayer != BUILDING) continue; // TODO: Wanted to query the collision matrix but is far too slow
            bool colHit = CheckBoxCollision(targetCol.worldBoundingBox, c.second->worldBoundingBox);
            if (colHit) return true;
        }
        return false;
    }

    std::vector<CollisionInfo> CollisionSystem::GetCollisions(EntityID entity)
    {
        std::vector<CollisionInfo> collisions;
        
        const Collideable& targetCol = *components.at(entity);

        for (const auto& c : components)
        {
            if (c.second->entityId == entity) continue;
            if (!checkCollisionMatrix(targetCol.collisionLayer, c.second->collisionLayer)) continue;
            
            bool colHit = CheckBoxCollision(targetCol.worldBoundingBox, c.second->worldBoundingBox);
            if (colHit)
            {
                CollisionInfo info = {
                    .collidedEntityId = c.second->entityId,
                    .collidedBB = c.second->worldBoundingBox
                };
                
                collisions.push_back(info);
            }
        }

        return collisions;
    }

    void CollisionSystem::DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data)
    {
        int id = std::stoi(entityId);
    
        auto localBoundingBoxMin = data.at("localBoundingBoxMin");
        auto localBoundingBoxMax = data.at("localBoundingBoxMax");
        auto worldBoundingBoxMin = data.at("worldBoundingBoxMin");
        auto worldBoundingBoxMax = data.at("worldBoundingBoxMax");
    
        BoundingBox localBoundingBox;
        localBoundingBox.min = Serializer::ConvertStringToVector3(localBoundingBoxMin);
        localBoundingBox.max = Serializer::ConvertStringToVector3(localBoundingBoxMax);
    
        BoundingBox worldBoundingBox;
        worldBoundingBox.min = Serializer::ConvertStringToVector3(worldBoundingBoxMin);
        worldBoundingBox.max = Serializer::ConvertStringToVector3(worldBoundingBoxMax);

        auto collideable = std::make_unique<Collideable>(id, localBoundingBox);
        collideable->worldBoundingBox = worldBoundingBox;
        collideable->collisionLayer = static_cast<CollisionLayer>(std::stoi(data.at("collisionLayer")));
        AddComponent(std::move(collideable));
    }

    void CollisionSystem::AddComponent(std::unique_ptr<Collideable> component)
    {
        if (ECS->transformSystem->FindEntity(component->entityId))
        {
            eventManager->Subscribe([p = this, id = component->entityId] { p->onTransformUpdate(id); },
                                    *ECS->transformSystem->GetComponent(component->entityId)->OnPositionUpdate);
        }

        m_addComponent(std::move(component));
    }

}