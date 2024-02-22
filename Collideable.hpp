//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Component.hpp"
#include "raylib.h"


namespace sage
{
    enum CollisionLayer
    {
        DEFAULT,
        FLOOR,
        BUILDING
    };

    struct Collideable : public Component
    {
        BoundingBox boundingBox{};
        CollisionLayer collisionLayer = DEFAULT;
        //Event OnCollisionHit;
        explicit Collideable(EntityID _entityId) : Component(_entityId) {}
    };

    struct CollisionInfo
    {
        EntityID collidedEntityId{};
        RayCollision rayCollision{};
    };
}

