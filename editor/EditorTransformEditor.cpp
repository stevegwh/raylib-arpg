//
// EditorTransformEditor implementation. See EditorTransformEditor.hpp.
//

#include "EditorTransformEditor.hpp"

#include "EditorTransformMath.hpp"
#include "engine/Camera.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/Settings.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/NavigationGridSystem.hpp"

#include "raymath.h"

#include <algorithm>
#include <utility>

namespace sage::editor
{
    namespace
    {
        // Mirrors the floor used elsewhere in the editor when treating uniform
        // scale as a single number. Kept local; placement-grid extraction will
        // eventually centralise this.
        constexpr float MIN_SCALE = 0.1f;
    } // namespace

    EditorTransformEditor::EditorTransformEditor(EngineSystems* _sys, OnApplied _onApplied)
        : sys(_sys), onApplied(std::move(_onApplied))
    {
    }

    void EditorTransformEditor::EnterEditMode(const entt::entity entity, EditorEditState& outSnapshot)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        const auto& transform = sys->registry->get<sgTransform>(entity);
        outSnapshot.entity = entity;
        outSnapshot.originalPosition = transform.GetWorldPos();
        outSnapshot.originalRotation = transform.GetWorldRot();
        outSnapshot.originalScale = transform.GetScale();
    }

    void EditorTransformEditor::ExitEditMode()
    {
        if (gizmo.IsDragging())
        {
            sys->camera->UnlockInput();
        }
        gizmo.EndDrag();
    }

    void EditorTransformEditor::RestoreSnapshot(const EditorEditState& snapshot)
    {
        const auto entity = snapshot.entity;
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        auto& transform = sys->registry->get<sgTransform>(entity);
        transform.position.world = snapshot.originalPosition;
        transform.rotation.world = snapshot.originalRotation;
        transform.scale.world = snapshot.originalScale;

        updateEntityCollisionBounds(entity);
        notify(entity);
    }

    void EditorTransformEditor::Update(const entt::entity entity)
    {
        if (!gizmo.IsDragging()) return;

        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) || !sys->registry->valid(entity) ||
            !sys->registry->any_of<sgTransform>(entity))
        {
            ExitEditMode();
            return;
        }

        const Camera3D camera = *sys->camera->getRaylibCam();
        const Vector2 viewport = sys->settings->GetRenderViewPort();
        const Vector2 mousePosition = sys->settings->ScreenToRenderViewportPosition(GetMousePosition());
        const Vector3 origin = PivotWorldPosition(entity);

        const auto sample = gizmo.SampleDrag(camera, viewport, origin, mode, mousePosition);
        if (sample.axis == EditGizmo::Axis::None) return;

        switch (mode)
        {
        case EditGizmo::Mode::Translate:
        {
            // Convert the projected screen-pixel delta back into world units by
            // walking the same screen-axis-length the gizmo used for projection.
            const Vector3 axisVector = EditGizmo::AxisVector(sample.axis);
            const float size = EditGizmo::SizeForCamera(camera.position, origin);
            const Vector2 screenStart = GetWorldToScreenEx(
                origin, camera, static_cast<int>(viewport.x), static_cast<int>(viewport.y));
            const Vector2 screenEnd = GetWorldToScreenEx(
                Vector3Add(origin, Vector3Scale(axisVector, size)),
                camera,
                static_cast<int>(viewport.x),
                static_cast<int>(viewport.y));
            const float screenLength = Vector2Length(Vector2Subtract(screenEnd, screenStart));
            if (screenLength > 0.0001f)
            {
                AdjustPosition(entity, Vector3Scale(axisVector, sample.projectedAxisPixels * size / screenLength));
            }
            break;
        }
        case EditGizmo::Mode::Rotate:
            AdjustRotationAxis(entity, sample.axis, sample.rotationDegrees);
            break;
        case EditGizmo::Mode::Scale:
            AdjustScale(entity, sample.projectedAxisPixels * 0.01f);
            break;
        }
    }

    bool EditorTransformEditor::TryStartDrag(const entt::entity entity, const Vector2 mousePosition)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return false;
        if (!sys->settings->IsPointInRenderViewport(mousePosition)) return false;

        const Camera3D camera = *sys->camera->getRaylibCam();
        const Vector2 viewport = sys->settings->GetRenderViewPort();
        const Vector2 renderMousePosition = sys->settings->ScreenToRenderViewportPosition(mousePosition);
        const Vector3 origin = PivotWorldPosition(entity);

        const auto axis = gizmo.HitTest(camera, viewport, origin, mode, renderMousePosition);
        if (axis == EditGizmo::Axis::None) return false;

        gizmo.BeginDrag(axis, renderMousePosition);
        sys->camera->LockInput();
        return true;
    }

    void EditorTransformEditor::Draw3D(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        const Camera3D camera = *sys->camera->getRaylibCam();
        const Vector2 viewport = sys->settings->GetRenderViewPort();
        const Vector3 origin = PivotWorldPosition(entity);
        gizmo.Draw(camera, viewport, origin, mode);
    }

    void EditorTransformEditor::SetMode(const EditGizmo::Mode newMode)
    {
        mode = newMode;
    }

    void EditorTransformEditor::TogglePivotMode()
    {
        pivotMode = pivotMode == PivotMode::LocalCenter ? PivotMode::World : PivotMode::LocalCenter;
    }

    std::string EditorTransformEditor::DescribeMode() const
    {
        switch (mode)
        {
        case EditGizmo::Mode::Translate:
            return "Translate";
        case EditGizmo::Mode::Rotate:
            return "Rotate";
        case EditGizmo::Mode::Scale:
            return "Scale";
        }
        return "Translate";
    }

    Vector3 EditorTransformEditor::PivotWorldPosition(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return Vector3Zero();

        const auto& transform = sys->registry->get<sgTransform>(entity);
        if (pivotMode != PivotMode::LocalCenter || !sys->registry->any_of<Renderable>(entity))
        {
            return transform.GetWorldPos();
        }

        const auto& renderable = sys->registry->get<Renderable>(entity);
        const auto* model = renderable.GetModel();
        if (model == nullptr) return transform.GetWorldPos();

        const Matrix entityMatrix = BuildRenderableEntityMatrix(
            transform.GetWorldPos(), transform.GetWorldRot(), transform.GetScale());
        const BoundingBox worldBounds = TransformBoundingBoxByCorners(model->CalcLocalBoundingBox(), entityMatrix);
        return BoundingBoxCenter(worldBounds);
    }

    void EditorTransformEditor::AdjustPosition(const entt::entity entity, const Vector3 worldDelta)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        auto& transform = sys->registry->get<sgTransform>(entity);
        transform.position.world = Vector3Add(transform.GetWorldPos(), worldDelta);
        updateEntityCollisionBounds(entity);
        notify(entity);
    }

    void EditorTransformEditor::AdjustRotationAxis(
        const entt::entity entity, const EditGizmo::Axis axis, const float amount)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        auto wrapDegrees = [](float degrees) {
            while (degrees >= 360.0f) degrees -= 360.0f;
            while (degrees < 0.0f) degrees += 360.0f;
            return degrees;
        };

        auto& transform = sys->registry->get<sgTransform>(entity);
        Vector3 rotation = transform.GetWorldRot();
        Matrix rotationDelta = MatrixIdentity();

        if (axis == EditGizmo::Axis::X)
        {
            rotation.x = wrapDegrees(rotation.x + amount);
            rotationDelta = MatrixRotateX(amount * DEG2RAD);
        }
        else if (axis == EditGizmo::Axis::Z)
        {
            rotation.z = wrapDegrees(rotation.z + amount);
            rotationDelta = MatrixRotateZ(amount * DEG2RAD);
        }
        else
        {
            rotation.y = wrapDegrees(rotation.y + amount);
            rotationDelta = MatrixRotateY(amount * DEG2RAD);
        }

        // For LocalCenter pivot mode, the model's visible center should stay anchored
        // at the pivot. Translate the transform origin so the (pivot → origin) offset
        // rotates with the delta — the model's body then orbits the pivot.
        if (pivotMode == PivotMode::LocalCenter)
        {
            const Vector3 pivot = PivotWorldPosition(entity);
            const Vector3 offset = Vector3Subtract(transform.GetWorldPos(), pivot);
            const Vector3 newOffset = Vector3Transform(offset, rotationDelta);
            transform.position.world = Vector3Add(pivot, newOffset);
        }

        transform.rotation.world = rotation;
        updateEntityCollisionBounds(entity);
        notify(entity);
    }

    void EditorTransformEditor::AdjustScale(const entt::entity entity, const float delta)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        auto& transform = sys->registry->get<sgTransform>(entity);
        const Vector3 currentScale = transform.GetScale();
        const float currentUniformScale =
            std::max(MIN_SCALE, (currentScale.x + currentScale.y + currentScale.z) / 3.0f);
        const float nextUniformScale = std::max(MIN_SCALE, currentUniformScale + delta);
        const float scaleFactor = nextUniformScale / currentUniformScale;

        // For LocalCenter pivot mode, scale the (pivot → origin) offset so the model
        // expands/contracts about the pivot rather than about the transform origin.
        if (pivotMode == PivotMode::LocalCenter)
        {
            const Vector3 pivot = PivotWorldPosition(entity);
            const Vector3 offset = Vector3Subtract(transform.GetWorldPos(), pivot);
            const Vector3 newOffset = Vector3Scale(offset, scaleFactor);
            transform.position.world = Vector3Add(pivot, newOffset);
        }

        transform.scale.world = {nextUniformScale, nextUniformScale, nextUniformScale};
        updateEntityCollisionBounds(entity);
        notify(entity);
    }

    void EditorTransformEditor::updateEntityCollisionBounds(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->all_of<sgTransform, Collideable>(entity)) return;

        const auto& transform = sys->registry->get<sgTransform>(entity);
        auto& collideable = sys->registry->get<Collideable>(entity);
        if (collideable.blocksNavigation)
        {
            sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false, entity);
        }

        const Matrix entityMatrix = BuildRenderableEntityMatrix(
            transform.GetWorldPos(), transform.GetWorldRot(), transform.GetScale());
        if (sys->registry->any_of<Renderable>(entity))
        {
            const auto& renderable = sys->registry->get<Renderable>(entity);
            if (const auto* model = renderable.GetModel(); model != nullptr)
            {
                collideable.localBoundingBox = model->CalcLocalBoundingBox();
            }
        }
        collideable.worldBoundingBox = TransformBoundingBoxByCorners(collideable.localBoundingBox, entityMatrix);

        if (collideable.blocksNavigation)
        {
            sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
        }
    }

    void EditorTransformEditor::notify(const entt::entity entity) const
    {
        if (onApplied) onApplied(entity);
    }
} // namespace sage::editor
