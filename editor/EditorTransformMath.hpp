//
// Matrix / bounding-box helpers shared by EditorScene (placement, picking,
// preview rendering) and EditorTransformEditor (gizmo + pivot math). Pulled
// into a header so both translation units agree on the same definitions.
//

#pragma once

#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <array>

namespace sage::editor
{
    inline BoundingBox TransformBoundingBoxByCorners(const BoundingBox& bounds, const Matrix& transform)
    {
        const std::array<Vector3, 8> corners = {
            Vector3{bounds.min.x, bounds.min.y, bounds.min.z},
            Vector3{bounds.min.x, bounds.min.y, bounds.max.z},
            Vector3{bounds.min.x, bounds.max.y, bounds.min.z},
            Vector3{bounds.min.x, bounds.max.y, bounds.max.z},
            Vector3{bounds.max.x, bounds.min.y, bounds.min.z},
            Vector3{bounds.max.x, bounds.min.y, bounds.max.z},
            Vector3{bounds.max.x, bounds.max.y, bounds.min.z},
            Vector3{bounds.max.x, bounds.max.y, bounds.max.z},
        };

        BoundingBox transformed{};
        transformed.min = transformed.max = Vector3Transform(corners.front(), transform);

        for (const auto& corner : corners)
        {
            const auto worldCorner = Vector3Transform(corner, transform);
            transformed.min.x = std::min(transformed.min.x, worldCorner.x);
            transformed.min.y = std::min(transformed.min.y, worldCorner.y);
            transformed.min.z = std::min(transformed.min.z, worldCorner.z);
            transformed.max.x = std::max(transformed.max.x, worldCorner.x);
            transformed.max.y = std::max(transformed.max.y, worldCorner.y);
            transformed.max.z = std::max(transformed.max.z, worldCorner.z);
        }

        return transformed;
    }

    inline Matrix BuildRenderableEntityMatrix(const Vector3 position, const Vector3 rotation, const Vector3 scale)
    {
        const Matrix rotationMatrix = MatrixMultiply(
            MatrixMultiply(MatrixRotateZ(rotation.z * DEG2RAD), MatrixRotateY(rotation.y * DEG2RAD)),
            MatrixRotateX(rotation.x * DEG2RAD));
        return MatrixMultiply(
            MatrixMultiply(MatrixScale(scale.x, scale.y, scale.z), rotationMatrix),
            MatrixTranslate(position.x, position.y, position.z));
    }

    inline Matrix BuildPivotDeltaMatrix(const Vector3 pivot, const Matrix& delta)
    {
        return MatrixMultiply(
            MatrixMultiply(MatrixTranslate(-pivot.x, -pivot.y, -pivot.z), delta),
            MatrixTranslate(pivot.x, pivot.y, pivot.z));
    }

    inline Vector3 BoundingBoxCenter(const BoundingBox& bounds)
    {
        return Vector3Scale(Vector3Add(bounds.min, bounds.max), 0.5f);
    }
} // namespace sage::editor
