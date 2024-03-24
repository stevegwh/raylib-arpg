//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <map>
#include <vector>
#include <utility>

#include "raylib.h"
#include "raymath.h"

#include "Collideable.hpp"
#include "BaseSystem.hpp"

namespace sage
{
class CollisionSystem : public BaseSystem<Collideable>
{
    bool checkCollisionMatrix(const CollisionLayer& layer1, const CollisionLayer& layer2);
public:
    CollisionSystem() : BaseSystem<Collideable>("Collideable") {}
    void UpdateWorldBoundingBox(EntityID entityId, Vector3 pos);
    [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(const Ray& ray) const;
    void BoundingBoxDraw(EntityID entityId, Color color = LIME) const;
    static bool CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2) ;
    [[nodiscard]] std::vector<CollisionInfo> GetCollisions(EntityID entity);
    //std::pair<bool, CollisionInfo> GetFirstCollision(EntityID entity);
    bool GetFirstCollision(EntityID entity);
    void DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data);
    
    const std::vector<std::vector<CollisionLayer>> collisionMatrix = 
        {
        {CollisionLayer::FLOOR, CollisionLayer::BUILDING}, // Default
        {CollisionLayer::FLOOR}, // Floor
        {CollisionLayer::BUILDING}, // Building
        {CollisionLayer::BUILDING}, // Navigation
        {CollisionLayer::FLOOR, CollisionLayer::BUILDING}, // Player
    };

};
}

