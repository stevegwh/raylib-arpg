//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "components/Collideable.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    using CollisionMatrix = std::vector<std::vector<bool>>;

    class CollisionSystem : public BaseSystem
    {
        [[nodiscard]] static CollisionMatrix CreateCollisionMatrix();

      public:
        CollisionMatrix collisionMatrix;

        explicit CollisionSystem(entt::registry* _registry);
        void DrawDebug();
        void OnTransformUpdate(entt::entity entity);
        void UpdateWorldBoundingBox(entt::entity entityId, Matrix mat);
        [[nodiscard]] std::vector<CollisionInfo> GetMeshCollisionsWithRay(
            const entt::entity& caster, const Ray& ray, CollisionLayer layer);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(
            const entt::entity& caster,
            const Ray& ray,
            CollisionLayer layer = CollisionLayer::DEFAULT);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(
            const Ray& ray, CollisionLayer layer = CollisionLayer::DEFAULT);
        [[nodiscard]] bool GetFirstCollisionWithRay(
            const Ray& ray,
            CollisionInfo& info,
            CollisionLayer layer = CollisionLayer::DEFAULT);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithBoundingBox(
            const BoundingBox& bb, CollisionLayer layer = CollisionLayer::DEFAULT);
        void BoundingBoxDraw(entt::entity entityId, Color color = LIME) const;
        static bool CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2);
        bool GetFirstCollision(entt::entity entity);
    };
} // namespace sage
