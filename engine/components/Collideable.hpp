//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "../CollisionLayers.hpp"

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"

#include "raylib.h"
#include "raymath.h"

namespace sage
{
    struct Collideable
    {
        BoundingBox localBoundingBox{};
        BoundingBox worldBoundingBox{};
        CollisionLayer collisionLayer = sage::collision_layers::Default;
        CollisionMask collidesWith = GetDefaultCollisionMask(sage::collision_layers::Default);
        bool active = true;
        bool isStatic = false;
        bool debugDraw = false;
        bool blocksNavigation = false;

        Collideable() = default;
        Collideable(const BoundingBox& local, const Matrix& worldMat);
        void SetCollisionLayer(CollisionLayer layer, CollisionMask mask = CollisionMask{});

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(localBoundingBox, worldBoundingBox, collisionLayer);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(localBoundingBox, worldBoundingBox, collisionLayer);
            collisionLayer.layerName = GetCollisionLayerName(collisionLayer.bit);
            collidesWith = GetDefaultCollisionMask(collisionLayer);
        }

        template <class Inspector>
        void define_editor_fields(Inspector& i)
        {
            i.field("Active", active);
            i.field("IsStatic", isStatic);
            i.field("Debug Draw", debugDraw);
            i.field("Blocks Navigation", blocksNavigation);
            i.field("Collision Layer", collisionLayer);
            i.field("Local Bounds", localBoundingBox);
            i.field("World Bounds", worldBoundingBox, false);
        }
    };

    // Transient ECS tag: when present, CollisionSystem ignores Collideable::isStatic
    // for bounds refreshes without changing the authored static flag.
    struct CollideableStaticOverride
    {
    };

    // Transforms a bounding box by a world matrix.
    BoundingBox TransformBoundingBox(const BoundingBox& local, const Matrix& worldMat);
    BoundingBox TransformBoundingBoxByCorners(const BoundingBox& local, const Matrix& worldMat);
} // namespace sage
