//
// EditGizmo implementation. See EditGizmo.hpp.
//

#include "EditGizmo.hpp"

#include "raymath.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace sage::editor
{
    namespace
    {
        constexpr float GIZMO_MIN_SIZE = 1.5f;
        constexpr float GIZMO_MAX_SIZE = 9.0f;
        constexpr float GIZMO_SCREEN_HIT_RADIUS = 12.0f;
        constexpr int GIZMO_RING_SEGMENTS = 72;

        Vector2 WorldToScreen(const Camera3D& camera, const Vector2 viewport, const Vector3 worldPosition)
        {
            return GetWorldToScreenEx(
                worldPosition, camera, static_cast<int>(viewport.x), static_cast<int>(viewport.y));
        }

        float DistancePointToSegment(const Vector2 point, const Vector2 start, const Vector2 end)
        {
            const Vector2 segment = Vector2Subtract(end, start);
            const float segmentLengthSquared = Vector2DotProduct(segment, segment);
            if (segmentLengthSquared <= 0.0001f) return Vector2Distance(point, start);

            const float t = std::clamp(
                Vector2DotProduct(Vector2Subtract(point, start), segment) / segmentLengthSquared, 0.0f, 1.0f);
            const Vector2 projection = Vector2Add(start, Vector2Scale(segment, t));
            return Vector2Distance(point, projection);
        }

        float ScreenDistanceToRotationRing(
            const Camera3D& camera,
            const Vector2 viewport,
            const Vector3 origin,
            const float radius,
            const EditGizmo::Axis axis,
            const Vector2 mousePosition)
        {
            float closestDistance = std::numeric_limits<float>::max();
            Vector2 previousScreen = WorldToScreen(camera, viewport, EditGizmo::RotationRingPoint(origin, radius, axis, 0.0f));

            for (int i = 1; i <= GIZMO_RING_SEGMENTS; ++i)
            {
                const float angle = (2.0f * PI * static_cast<float>(i)) / static_cast<float>(GIZMO_RING_SEGMENTS);
                const Vector2 currentScreen =
                    WorldToScreen(camera, viewport, EditGizmo::RotationRingPoint(origin, radius, axis, angle));
                closestDistance = std::min(
                    closestDistance, DistancePointToSegment(mousePosition, previousScreen, currentScreen));
                previousScreen = currentScreen;
            }

            return closestDistance;
        }

        float ProjectedMouseDelta(
            const Camera3D& camera,
            const Vector2 viewport,
            const Vector3 origin,
            const EditGizmo::Axis axis,
            const Vector2 mouseDelta)
        {
            const Vector3 axisVector = EditGizmo::AxisVector(axis);
            if (Vector3Length(axisVector) <= 0.0001f) return 0.0f;

            const float size = EditGizmo::SizeForCamera(camera.position, origin);
            const Vector2 start = WorldToScreen(camera, viewport, origin);
            const Vector2 end = WorldToScreen(camera, viewport, Vector3Add(origin, Vector3Scale(axisVector, size)));
            const Vector2 screenAxis = Vector2Subtract(end, start);
            const float screenLength = Vector2Length(screenAxis);
            if (screenLength <= 0.0001f) return 0.0f;

            return Vector2DotProduct(mouseDelta, Vector2Scale(screenAxis, 1.0f / screenLength));
        }
    } // namespace

    Vector3 EditGizmo::AxisVector(const Axis axis)
    {
        switch (axis)
        {
        case Axis::X:
            return {1.0f, 0.0f, 0.0f};
        case Axis::Y:
            return {0.0f, 1.0f, 0.0f};
        case Axis::Z:
            return {0.0f, 0.0f, 1.0f};
        case Axis::None:
        case Axis::Uniform:
            return Vector3Zero();
        }
        return Vector3Zero();
    }

    Color EditGizmo::AxisColor(const Axis axis)
    {
        switch (axis)
        {
        case Axis::X:
            return RED;
        case Axis::Y:
            return GREEN;
        case Axis::Z:
            return BLUE;
        case Axis::Uniform:
            return GOLD;
        case Axis::None:
            return ORANGE;
        }
        return ORANGE;
    }

    Vector3 EditGizmo::RotationRingPoint(
        const Vector3 origin, const float radius, const Axis axis, const float angleRad)
    {
        switch (axis)
        {
        case Axis::X:
            return Vector3Add(origin, {0.0f, std::cos(angleRad) * radius, std::sin(angleRad) * radius});
        case Axis::Y:
            return Vector3Add(origin, {std::cos(angleRad) * radius, 0.0f, std::sin(angleRad) * radius});
        case Axis::Z:
            return Vector3Add(origin, {std::cos(angleRad) * radius, std::sin(angleRad) * radius, 0.0f});
        case Axis::None:
        case Axis::Uniform:
            return origin;
        }
        return origin;
    }

    float EditGizmo::SizeForCamera(const Vector3 cameraPosition, const Vector3 origin)
    {
        const float distance = Vector3Distance(cameraPosition, origin);
        return std::clamp(distance * 0.075f, GIZMO_MIN_SIZE, GIZMO_MAX_SIZE);
    }

    EditGizmo::Axis EditGizmo::HitTest(
        const Camera3D& camera,
        const Vector2 viewport,
        const Vector3 origin,
        const Mode mode,
        const Vector2 mousePosition) const
    {
        const float size = SizeForCamera(camera.position, origin);

        if (mode == Mode::Rotate)
        {
            Axis closestAxis = Axis::None;
            float closestDistance = std::numeric_limits<float>::max();
            for (const auto axis : {Axis::X, Axis::Y})
            {
                const float distance =
                    ScreenDistanceToRotationRing(camera, viewport, origin, size, axis, mousePosition);
                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    closestAxis = axis;
                }
            }
            return closestDistance <= GIZMO_SCREEN_HIT_RADIUS ? closestAxis : Axis::None;
        }

        if (mode == Mode::Scale &&
            Vector2Distance(mousePosition, WorldToScreen(camera, viewport, origin)) <= GIZMO_SCREEN_HIT_RADIUS)
        {
            return Axis::Uniform;
        }

        Axis closestAxis = Axis::None;
        float closestDistance = std::numeric_limits<float>::max();
        for (const auto axis : {Axis::X, Axis::Y, Axis::Z})
        {
            const Vector3 end = Vector3Add(origin, Vector3Scale(AxisVector(axis), size));
            const float distance = DistancePointToSegment(
                mousePosition, WorldToScreen(camera, viewport, origin), WorldToScreen(camera, viewport, end));
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestAxis = axis;
            }
        }
        return closestDistance <= GIZMO_SCREEN_HIT_RADIUS ? closestAxis : Axis::None;
    }

    void EditGizmo::BeginDrag(const Axis axis, const Vector2 mousePosition)
    {
        drag = {.active = true, .axis = axis, .lastMousePosition = mousePosition};
    }

    void EditGizmo::EndDrag()
    {
        drag = {};
    }

    EditGizmo::DragSample EditGizmo::SampleDrag(
        const Camera3D& camera, const Vector2 viewport, const Vector3 origin, const Mode mode)
    {
        if (!drag.active) return {};

        const Vector2 previousMousePosition = drag.lastMousePosition;
        const Vector2 mousePosition = GetMousePosition();
        const Vector2 mouseDelta = Vector2Subtract(mousePosition, previousMousePosition);
        drag.lastMousePosition = mousePosition;
        if (Vector2Length(mouseDelta) <= 0.0001f) return {};

        DragSample sample{.axis = drag.axis, .mouseDelta = mouseDelta};

        switch (mode)
        {
        case Mode::Translate:
            sample.projectedAxisPixels = ProjectedMouseDelta(camera, viewport, origin, drag.axis, mouseDelta);
            break;
        case Mode::Rotate:
        {
            const Vector2 center = WorldToScreen(camera, viewport, origin);
            const Vector2 previousVector = Vector2Subtract(previousMousePosition, center);
            const Vector2 currentVector = Vector2Subtract(mousePosition, center);
            if (Vector2Length(previousVector) > 0.0001f && Vector2Length(currentVector) > 0.0001f)
            {
                float deltaDegrees =
                    (std::atan2(currentVector.y, currentVector.x) -
                     std::atan2(previousVector.y, previousVector.x)) *
                    RAD2DEG;
                if (deltaDegrees > 180.0f) deltaDegrees -= 360.0f;
                if (deltaDegrees < -180.0f) deltaDegrees += 360.0f;
                sample.rotationDegrees = deltaDegrees;
            }
            break;
        }
        case Mode::Scale:
            sample.projectedAxisPixels = drag.axis == Axis::Uniform
                                             ? -mouseDelta.y
                                             : ProjectedMouseDelta(camera, viewport, origin, drag.axis, mouseDelta);
            break;
        }

        return sample;
    }

    void EditGizmo::Draw(
        const Camera3D& camera,
        const Vector2 /*viewport*/,
        const Vector3 origin,
        const Mode mode) const
    {
        const float size = SizeForCamera(camera.position, origin);
        const float shaftRadius = size * 0.012f;
        const float handleSize = size * 0.11f;

        DrawSphere(origin, size * 0.035f, ORANGE);

        const auto axisColor = [this](const Axis axis) {
            return drag.active && drag.axis == axis ? GOLD : AxisColor(axis);
        };

        const auto drawTranslateAxis = [&](const Axis axis) {
            const Vector3 axisVector = AxisVector(axis);
            const Vector3 end = Vector3Add(origin, Vector3Scale(axisVector, size));
            const Vector3 coneBase = Vector3Add(origin, Vector3Scale(axisVector, size * 0.78f));
            const Color color = axisColor(axis);
            DrawCylinderEx(origin, coneBase, shaftRadius, shaftRadius, 8, color);
            DrawCylinderEx(coneBase, end, size * 0.055f, 0.0f, 12, color);
        };

        const auto drawScaleAxis = [&](const Axis axis) {
            const Vector3 axisVector = AxisVector(axis);
            const Vector3 end = Vector3Add(origin, Vector3Scale(axisVector, size));
            const Color color = axisColor(axis);
            DrawCylinderEx(origin, end, shaftRadius, shaftRadius, 8, color);
            DrawCubeV(end, {handleSize, handleSize, handleSize}, color);
        };

        switch (mode)
        {
        case Mode::Translate:
            drawTranslateAxis(Axis::X);
            drawTranslateAxis(Axis::Y);
            drawTranslateAxis(Axis::Z);
            break;
        case Mode::Rotate:
        {
            const auto drawRotationRing = [&](const Axis axis) {
                const Color ringColor = axisColor(axis);
                Vector3 previous = RotationRingPoint(origin, size, axis, 0.0f);
                for (int i = 1; i <= GIZMO_RING_SEGMENTS; ++i)
                {
                    const float angle =
                        (2.0f * PI * static_cast<float>(i)) / static_cast<float>(GIZMO_RING_SEGMENTS);
                    const Vector3 current = RotationRingPoint(origin, size, axis, angle);
                    DrawLine3D(previous, current, ringColor);
                    previous = current;
                }
            };
            drawRotationRing(Axis::X);
            drawRotationRing(Axis::Y);
            break;
        }
        case Mode::Scale:
            drawScaleAxis(Axis::X);
            drawScaleAxis(Axis::Y);
            drawScaleAxis(Axis::Z);
            DrawCubeV(
                origin,
                {handleSize * 0.9f, handleSize * 0.9f, handleSize * 0.9f},
                drag.active && drag.axis == Axis::Uniform ? GOLD : Color{245, 245, 245, 255});
            break;
        }
    }
} // namespace sage::editor
