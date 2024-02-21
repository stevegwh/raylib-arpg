//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include <raylib.h>
#include "Event.hpp"
#include "Renderable.hpp"

namespace sage
{
    enum CollisionLayer
    {
        DEFAULT,
        FLOOR,
        BUILDING
    };

    struct Collideable
    {
        // Entity GUID
        BoundingBox boundingBox{};
        CollisionLayer collisionLayer = DEFAULT;
        //Event OnCollisionHit;
    };

    struct CollisionInfo
    {
        Collideable collidedObject;
        RayCollision rayCollision{};
    };
}

