//
// Created by Steve Wheeler on 02/05/2024.
//

#include "Collideable.hpp"

#include "raymath.h"

#include <algorithm>
#include <array>

namespace sage
{
    BoundingBox TransformBoundingBox(const BoundingBox& local, const Matrix& worldMat)
    {
        return {Vector3Transform(local.min, worldMat), Vector3Transform(local.max, worldMat)};
    }

    BoundingBox TransformBoundingBoxByCorners(const BoundingBox& local, const Matrix& worldMat)
    {
        const std::array<Vector3, 8> corners = {
            Vector3{local.min.x, local.min.y, local.min.z},
            Vector3{local.min.x, local.min.y, local.max.z},
            Vector3{local.min.x, local.max.y, local.min.z},
            Vector3{local.min.x, local.max.y, local.max.z},
            Vector3{local.max.x, local.min.y, local.min.z},
            Vector3{local.max.x, local.min.y, local.max.z},
            Vector3{local.max.x, local.max.y, local.min.z},
            Vector3{local.max.x, local.max.y, local.max.z},
        };

        BoundingBox transformed{};
        transformed.min = transformed.max = Vector3Transform(corners.front(), worldMat);

        for (const auto& corner : corners)
        {
            const auto worldCorner = Vector3Transform(corner, worldMat);
            transformed.min.x = std::min(transformed.min.x, worldCorner.x);
            transformed.min.y = std::min(transformed.min.y, worldCorner.y);
            transformed.min.z = std::min(transformed.min.z, worldCorner.z);
            transformed.max.x = std::max(transformed.max.x, worldCorner.x);
            transformed.max.y = std::max(transformed.max.y, worldCorner.y);
            transformed.max.z = std::max(transformed.max.z, worldCorner.z);
        }

        return transformed;
    }

    Collideable::Collideable(const BoundingBox& local, const Matrix& worldMat)
    {
        localBoundingBox = local;
        worldBoundingBox = TransformBoundingBox(local, worldMat);
    }

    void Collideable::SetCollisionLayer(const CollisionLayer layer, const CollisionMask mask)
    {
        collisionLayer = layer;
        collidesWith = mask.IsEmpty() ? GetDefaultCollisionMask(layer) : mask;
    }
} // namespace sage
