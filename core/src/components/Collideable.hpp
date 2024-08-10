//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    enum class CollisionLayer
    {
        DEFAULT,
        FLOOR,
        BUILDING,
        NAVIGATION,
        PLAYER,
        NPC,
        ENEMY,
        BOYD,
        TERRAIN,
        COUNT // Must always be last
    };

    struct CollisionInfo
    {
        entt::entity collidedEntityId{};
        BoundingBox collidedBB{};
        RayCollision rlCollision{};
        CollisionLayer collisionLayer{};
    };

    class Collideable
    {
        entt::registry* registry{};

      public:
        void OnTransformUpdate(entt::entity entity);
        BoundingBox localBoundingBox{}; // BoundingBox in local space
        BoundingBox worldBoundingBox{}; // BoundingBox in world space (bb * world mat)

        CollisionLayer collisionLayer = CollisionLayer::DEFAULT;
        bool debugDraw = false;

        void SetWorldBoundingBox(Matrix mat);

        // Static object
        explicit Collideable(BoundingBox _boundingBox);
        // Dynamic object
        Collideable(
            entt::registry* _registry, entt::entity _self, BoundingBox _boundingBox);
        // Static object (required for serialization)
        Collideable() = default;

        Collideable(const Collideable&) = delete;
        Collideable& operator=(const Collideable&) = delete;

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(localBoundingBox, worldBoundingBox, collisionLayer);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(localBoundingBox, worldBoundingBox, collisionLayer);
        }
    };
} // namespace sage
