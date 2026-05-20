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
        void inspect(Inspector& inspector)
        {
            inspector.Bool("Active", active);
            inspector.Bool("Debug Draw", debugDraw);
            inspector.Bool("Blocks Navigation", blocksNavigation);
            inspector.CollisionLayer("Collision Layer", collisionLayer);
            inspector.Vec3("Local Bounds Min", localBoundingBox.min);
            inspector.Vec3("Local Bounds Max", localBoundingBox.max);
            inspector.Vec3("World Bounds Min", worldBoundingBox.min, false);
            inspector.Vec3("World Bounds Max", worldBoundingBox.max, false);
        }
    };

    // Empty tag. Attach alongside Collideable to opt out of CollisionSystem::Update —
    // the entity's worldBoundingBox is baked at construction (or load) and never
    // recomputed. Matches the RenderableDeferred convention.
    struct StaticCollideable
    {
    };

    // Transforms a bounding box by a world matrix.
    BoundingBox TransformBoundingBox(const BoundingBox& local, const Matrix& worldMat);
} // namespace sage
