//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"
#include "raylib.h"
#include "raymath.h"

namespace sage
{
    enum class CollisionLayer
    {
        DEFAULT,
        FLOORSIMPLE,
        BUILDING,
        NAVIGATION,
        PLAYER,
        NPC,
        ENEMY,
        BOYD,
        FLOORCOMPLEX,
        BACKGROUND,
        STAIRS,
        ITEM,
        COUNT // Must always be last
    };

    struct CollisionInfo
    {
        entt::entity collidedEntityId{};
        BoundingBox collidedBB{};
        RayCollision rlCollision{};
        CollisionLayer collisionLayer{};
    };

    struct CollisionChecker
    {
        float maxDistance = 0;
        Vector3 origin;
        Vector3 destination;
        entt::sigh<void(entt::entity, CollisionInfo)> onHit;
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
        explicit Collideable(BoundingBox _localBoundingBox, Matrix worldMatrix);
        // Dynamic object
        Collideable(entt::registry* _registry, entt::entity _self, BoundingBox _localBoundingBox);
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
