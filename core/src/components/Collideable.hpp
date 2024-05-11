//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "raylib.h"
#include "entt/entt.hpp"
#include "cereal/cereal.hpp"

namespace sage
{

enum CollisionLayer
{
    DEFAULT,
    FLOOR,
    BUILDING,
    NAVIGATION,
    PLAYER,
    NPC
};

struct CollisionInfo
{
    entt::entity collidedEntityId{};
    BoundingBox collidedBB{};
    RayCollision rlCollision{};
};

struct Collideable
{
    BoundingBox localBoundingBox{}; // BoundingBox in local space
    BoundingBox worldBoundingBox{}; // BoundingBox in world space (bb * world mat)
    CollisionLayer collisionLayer = DEFAULT;

    explicit Collideable(BoundingBox _boundingBox);
    Collideable() = default;
    Collideable(const Collideable&) = delete;
    Collideable& operator=(const Collideable&) = delete;

    template<class Archive>
    void save(Archive & archive) const
    {
        archive(
                CEREAL_NVP(localBoundingBox.min.x),
                CEREAL_NVP(localBoundingBox.min.y),
                CEREAL_NVP(localBoundingBox.min.z),
                CEREAL_NVP(localBoundingBox.max.x),
                CEREAL_NVP(localBoundingBox.max.y),
                CEREAL_NVP(localBoundingBox.max.z),
                CEREAL_NVP(worldBoundingBox.min.x),
                CEREAL_NVP(worldBoundingBox.min.y),
                CEREAL_NVP(worldBoundingBox.min.z),
                CEREAL_NVP(worldBoundingBox.max.x),
                CEREAL_NVP(worldBoundingBox.max.y),
                CEREAL_NVP(worldBoundingBox.max.z),
                CEREAL_NVP(collisionLayer));
    }

    template<class Archive>
    void load(Archive & archive)
    {
        archive(
                CEREAL_NVP(localBoundingBox.min.x),
                CEREAL_NVP(localBoundingBox.min.y),
                CEREAL_NVP(localBoundingBox.min.z),
                CEREAL_NVP(localBoundingBox.max.x),
                CEREAL_NVP(localBoundingBox.max.y),
                CEREAL_NVP(localBoundingBox.max.z),
                CEREAL_NVP(worldBoundingBox.min.x),
                CEREAL_NVP(worldBoundingBox.min.y),
                CEREAL_NVP(worldBoundingBox.min.z),
                CEREAL_NVP(worldBoundingBox.max.x),
                CEREAL_NVP(worldBoundingBox.max.y),
                CEREAL_NVP(worldBoundingBox.max.z),
                CEREAL_NVP(collisionLayer));
    }

};


}

