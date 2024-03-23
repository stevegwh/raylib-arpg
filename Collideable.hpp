//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Component.hpp"

#include "raylib.h"



#include <unordered_map>


namespace sage
{
    enum CollisionLayer
    {
        DEFAULT,
        FLOOR,
        BUILDING,
        NAVIGATION,
        PLAYER
    };

    struct Collideable : public Component<Collideable>
    {
    private:

    public:
        const BoundingBox localBoundingBox; // BoundingBox in local space
        BoundingBox worldBoundingBox{}; // BoundingBox in world space (bb pos + world pos)
        CollisionLayer collisionLayer = DEFAULT;
        explicit Collideable(EntityID _entityId, BoundingBox _boundingBox) :
            Component(_entityId), localBoundingBox(_boundingBox), worldBoundingBox(_boundingBox)
            {}

        [[nodiscard]] std::unordered_map<std::string, std::string> SerializeImpl() const
        {
            return {
                {"EntityId", TextFormat("%i", entityId)},
                {"localBoundingBoxMin", TextFormat("%02.02f, %02.02f, %02.02f", localBoundingBox.min.x, localBoundingBox.min.y, localBoundingBox.min.z)},
                {"localBoundingBoxMax", TextFormat("%02.02f, %02.02f, %02.02f", localBoundingBox.max.x, localBoundingBox.max.y, localBoundingBox.max.z)},
                {"worldBoundingBoxMin", TextFormat("%02.02f, %02.02f, %02.02f", worldBoundingBox.min.x, worldBoundingBox.min.y, worldBoundingBox.min.z)},
                {"worldBoundingBoxMax", TextFormat("%02.02f, %02.02f, %02.02f", worldBoundingBox.max.x, worldBoundingBox.max.y, worldBoundingBox.max.z)},
                {"collisionLayer", TextFormat("%i", collisionLayer)}
            };
        }
    };

    struct CollisionInfo
    {
        EntityID collidedEntityId{};
        BoundingBox collidedBB{};
        RayCollision rayCollision{};
    };
}

