//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Collideable.hpp"
#include "BaseSystem.hpp"

#include "raylib.h"
#include "raymath.h"
#include <entt/entt.hpp>

#include <map>
#include <vector>
#include <utility>

namespace sage
{
class CollisionSystem : public BaseSystem<Collideable>
{
    bool checkCollisionMatrix(const CollisionLayer& layer1, const CollisionLayer& layer2);
    void onTransformUpdate(entt::entity entityId);
public:
    explicit CollisionSystem(entt::registry* _registry);
    void UpdateWorldBoundingBox(entt::entity entityId, Matrix mat);
    [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(const Ray& ray) const;
    void BoundingBoxDraw(entt::entity entityId, Color color = LIME) const;
    static bool CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2) ;
    [[nodiscard]] std::vector<CollisionInfo> GetCollisions(entt::entity entity);
    //std::pair<bool, CollisionInfo> GetFirstCollision(EntityID entity);
    bool GetFirstCollision(entt::entity entity);
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

