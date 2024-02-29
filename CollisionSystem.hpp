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
    bool checkCollisionMatrix(CollisionLayer layer1, CollisionLayer layer2);
public:
    void SetBoundingBox(EntityID entityId, BoundingBox bb);
    [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(const Ray& ray) const;
    void BoundingBoxDraw(EntityID entityId, Color color = LIME) const;
    static bool CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2) ;
    [[nodiscard]] std::vector<CollisionInfo> GetCollisions(EntityID entity);
    //std::pair<bool, CollisionInfo> GetFirstCollision(EntityID entity);
    bool GetFirstCollision(EntityID entity);
    
    const std::unordered_map<CollisionLayer, std::vector<CollisionLayer>> collisionMatrix = {
        {CollisionLayer::DEFAULT, {CollisionLayer::FLOOR, CollisionLayer::BUILDING}},
        {CollisionLayer::FLOOR, {CollisionLayer::BUILDING}},
        {CollisionLayer::BUILDING, {CollisionLayer::FLOOR}},
        {CollisionLayer::NAVIGATION, {CollisionLayer::BUILDING}},
        {CollisionLayer::PLAYER, {CollisionLayer::FLOOR, CollisionLayer::BUILDING}},
    };

};
}

