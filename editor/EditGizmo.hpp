//
// The 3D translate / rotate / scale gizmo that hovers over the selected
// scene object while the editor is in edit mode. Owns its own drag
// state and the screen-space hit testing — the editor supplies the pivot
// point (which depends on the registry-driven pivot mode) and applies the
// delta the gizmo reports back.
//

#pragma once

#include "raylib.h"

namespace sage::editor
{
    class EditGizmo
    {
      public:
        enum class Axis
        {
            None,
            X,
            Y,
            Z,
            Uniform
        };

        enum class Mode
        {
            Translate,
            Rotate,
            Scale
        };

        // The result of polling a drag for one frame. Whatever the caller's
        // transform-mutation path is (registry write, pivot-aware matrix
        // composition, …), it consumes one of these fields based on the mode
        // the gizmo was sampled in:
        //   - Translate / non-uniform Scale: projectedAxisPixels along the
        //     screen-projected axis vector (positive = drag away from origin).
        //   - Uniform Scale: projectedAxisPixels = -mouseDelta.y so dragging
        //     upward grows the object.
        //   - Rotate: rotationDegrees around the screen vector from the gizmo
        //     origin to the cursor, normalised into [-180, 180].
        struct DragSample
        {
            Axis axis = Axis::None;
            Vector2 mouseDelta{};
            float projectedAxisPixels = 0.0f;
            float rotationDegrees = 0.0f;
        };

        // Mode-independent geometry. Exposed because EditorScene composes them
        // in its own pivot-aware transform math (e.g. rotating a renderable
        // around its bounding-box centre).
        static Vector3 AxisVector(Axis axis);
        static Color AxisColor(Axis axis);
        static Vector3 RotationRingPoint(Vector3 origin, float radius, Axis axis, float angleRad);
        static float SizeForCamera(Vector3 cameraPosition, Vector3 origin);

        // Returns the axis under the cursor for the given mode, or Axis::None.
        [[nodiscard]] Axis HitTest(
            const Camera3D& camera, Vector2 viewport, Vector3 origin, Mode mode, Vector2 mousePosition) const;

        // Drag lifecycle. The caller drives these from mouse-down / mouse-up
        // events and routes SampleDrag's output back into its transform math.
        void BeginDrag(Axis axis, Vector2 mousePosition);
        void EndDrag();
        [[nodiscard]] bool IsDragging() const
        {
            return drag.active;
        }
        [[nodiscard]] Axis DragAxis() const
        {
            return drag.axis;
        }

        // Returns DragSample{axis = Axis::None, …} when not dragging, when the
        // mouse hasn't moved, or when the projected screen vector is degenerate
        // (camera looking straight down the dragged axis). Otherwise updates
        // lastMousePosition so the next call samples the next delta.
        DragSample SampleDrag(const Camera3D& camera, Vector2 viewport, Vector3 origin, Mode mode);

        void Draw(const Camera3D& camera, Vector2 viewport, Vector3 origin, Mode mode) const;

      private:
        struct DragState
        {
            bool active = false;
            Axis axis = Axis::None;
            Vector2 lastMousePosition{};
        };

        DragState drag;
    };
} // namespace sage::editor
