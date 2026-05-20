//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "engine/components/Collideable.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    struct CollisionInfo
    {
        entt::entity collidedEntityId{};
        BoundingBox collidedBB{};
        RayCollision rlCollision{};
        CollisionLayer collisionLayer{};
    };

    class CollisionSystem
    {
        entt::registry* registry;
        CollisionMask defaultQueryMask{collision_masks::DefaultQuery};
        [[nodiscard]] CollisionMask ResolveQueryMask(CollisionLayer layer) const;

      public:
        // Recomputes worldBoundingBox for every dynamic Collideable from its sgTransform.
        // Static collideables are not visited (their world bbox is baked at construction).
        // Call once per frame, after positions have been mutated and before any queries.
        void Update() const;

        static void SortCollisionsByDistance(std::vector<CollisionInfo>& collisions);
        [[nodiscard]] std::vector<CollisionInfo> GetMeshCollisionsWithRay(
            const entt::entity& caster, const Ray& ray, CollisionLayer layer);
        [[nodiscard]] std::vector<CollisionInfo> GetMeshCollisionsWithRay(
            const entt::entity& caster, const Ray& ray, CollisionMask mask);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(
            const entt::entity& caster, const Ray& ray, CollisionLayer layer = sage::collision_layers::Default);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(
            const entt::entity& caster, const Ray& ray, CollisionMask mask);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(
            const Ray& ray, CollisionLayer layer = sage::collision_layers::Default);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithRay(const Ray& ray, CollisionMask mask);
        [[nodiscard]] bool GetFirstCollisionWithRay(
            const Ray& ray, CollisionInfo& info, CollisionLayer layer = sage::collision_layers::Default) const;
        [[nodiscard]] bool GetFirstCollisionWithRay(const Ray& ray, CollisionInfo& info, CollisionMask mask) const;
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithBoundingBox(
            const BoundingBox& bb, CollisionLayer layer = sage::collision_layers::Default);
        [[nodiscard]] std::vector<CollisionInfo> GetCollisionsWithBoundingBox(
            const BoundingBox& bb, CollisionMask mask);
        void BoundingBoxDraw(entt::entity entityId, Color color = LIME) const;
        static bool CheckBoxCollision(const BoundingBox& col1, const BoundingBox& col2);
        bool GetFirstCollisionBB(entt::entity caller, BoundingBox bb, CollisionLayer layer, CollisionInfo& out);
        bool GetFirstCollisionBB(
            entt::entity caller, BoundingBox bb, CollisionMask mask, CollisionInfo& out) const;
        void SetDefaultQueryMask(CollisionMask mask);
        void DrawDebug() const;
        explicit CollisionSystem(entt::registry* _registry);
    };
} // namespace sage
