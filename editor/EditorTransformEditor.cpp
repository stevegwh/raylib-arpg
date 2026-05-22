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
#include "engine/systems/TransformSystem.hpp"

#include "raymath.h"

#include <algorithm>
#include <cmath>
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

        if (sys->registry->any_of<Collideable>(entity))
        {
            const auto& collideable = sys->registry->get<Collideable>(entity);
            outSnapshot.originalLocalBoundingBox = collideable.localBoundingBox;
            outSnapshot.originalWorldBoundingBox = collideable.worldBoundingBox;
            outSnapshot.hadCollideable = true;
        }
        if (sys->registry->any_of<Renderable>(entity))
        {
            const auto& renderable = sys->registry->get<Renderable>(entity);
            if (const auto* model = renderable.GetModel(); model != nullptr)
            {
                outSnapshot.originalRenderableTransform = model->GetTransform();
                outSnapshot.originalRenderableInitialTransform = renderable.initialTransform;
                outSnapshot.hadRenderable = true;
            }
        }
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

        if (sys->registry->any_of<Collideable>(entity))
        {
            auto& collideable = sys->registry->get<Collideable>(entity);
            if (collideable.blocksNavigation)
            {
                sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false, entity);
            }
        }

        sys->registry->get<sage::sgTransform>(entity).SetWorldPos(snapshot.originalPosition);
        sys->registry->get<sage::sgTransform>(entity).SetWorldRot(snapshot.originalRotation);
        sys->registry->get<sage::sgTransform>(entity).SetWorldScale(snapshot.originalScale);

        if (snapshot.hadRenderable && sys->registry->any_of<Renderable>(entity))
        {
            auto& renderable = sys->registry->get<Renderable>(entity);
            if (auto* model = renderable.GetModel(); model != nullptr)
            {
                model->SetTransform(snapshot.originalRenderableTransform);
                renderable.initialTransform = snapshot.originalRenderableInitialTransform;
            }
        }

        if (snapshot.hadCollideable && sys->registry->any_of<Collideable>(entity))
        {
            auto& collideable = sys->registry->get<Collideable>(entity);
            collideable.localBoundingBox = snapshot.originalLocalBoundingBox;
            collideable.worldBoundingBox = snapshot.originalWorldBoundingBox;
            if (collideable.blocksNavigation)
            {
                sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
            }
        }
        else
        {
            updateEntityCollisionBounds(entity);
        }

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

        const auto& transform = sys->registry->get<sgTransform>(entity);
        const Vector3 position = Vector3Add(transform.GetWorldPos(), worldDelta);
        sys->registry->get<sage::sgTransform>(entity).SetWorldPos(position);
        updateEntityCollisionBounds(entity);
        notify(entity);
    }

    void EditorTransformEditor::AdjustRotationAxis(
        const entt::entity entity, const EditGizmo::Axis axis, const float amount)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        const auto& transform = sys->registry->get<sgTransform>(entity);
        const Vector3 position = transform.GetWorldPos();
        const Vector3 scale = transform.GetScale();
        Vector3 rotation = transform.GetWorldRot();
        auto wrapDegrees = [](float degrees) {
            while (degrees >= 360.0f) degrees -= 360.0f;
            while (degrees < 0.0f) degrees += 360.0f;
            return degrees;
        };

        if (axis == EditGizmo::Axis::X)
        {
            rotation.x = wrapDegrees(rotation.x + amount);
        }
        else if (axis == EditGizmo::Axis::Z)
        {
            rotation.z = wrapDegrees(rotation.z + amount);
        }
        else
        {
            rotation.y = wrapDegrees(rotation.y + amount);
        }

        if (pivotMode == PivotMode::LocalCenter && sys->registry->any_of<Renderable>(entity))
        {
            const auto& renderable = sys->registry->get<Renderable>(entity);
            if (const auto* model = renderable.GetModel(); model != nullptr)
            {
                const Matrix oldEntityMatrix =
                    BuildRenderableEntityMatrix(position, transform.GetWorldRot(), scale);
                const Matrix oldWorldMatrix = MatrixMultiply(model->GetTransform(), oldEntityMatrix);
                const Vector3 pivot = PivotWorldPosition(entity);
                Matrix rotationDelta = MatrixIdentity();
                if (axis == EditGizmo::Axis::X)
                {
                    rotationDelta = MatrixRotateX(amount * DEG2RAD);
                }
                else if (axis == EditGizmo::Axis::Z)
                {
                    rotationDelta = MatrixRotateZ(amount * DEG2RAD);
                }
                else
                {
                    rotationDelta = MatrixRotateY(amount * DEG2RAD);
                }
                const Matrix desiredWorldMatrix =
                    MatrixMultiply(oldWorldMatrix, BuildPivotDeltaMatrix(pivot, rotationDelta));
                applyWorldMatrix(entity, desiredWorldMatrix, position, rotation, scale);
            }
            else
            {
                sys->registry->get<sage::sgTransform>(entity).SetWorldRot(rotation);
                updateEntityCollisionBounds(entity);
            }
        }
        else
        {
            sys->registry->get<sage::sgTransform>(entity).SetWorldRot(rotation);
            updateEntityCollisionBounds(entity);
        }

        notify(entity);
    }

    void EditorTransformEditor::AdjustScale(const entt::entity entity, const float delta)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        const auto& transform = sys->registry->get<sgTransform>(entity);
        const Vector3 position = transform.GetWorldPos();
        const Vector3 rotation = transform.GetWorldRot();
        const Vector3 currentScale = transform.GetScale();
        const float currentUniformScale =
            std::max(MIN_SCALE, (currentScale.x + currentScale.y + currentScale.z) / 3.0f);
        const float nextUniformScale = std::max(MIN_SCALE, currentUniformScale + delta);
        const Vector3 nextScale{nextUniformScale, nextUniformScale, nextUniformScale};

        if (pivotMode == PivotMode::LocalCenter && sys->registry->any_of<Renderable>(entity))
        {
            const auto& renderable = sys->registry->get<Renderable>(entity);
            if (const auto* model = renderable.GetModel(); model != nullptr)
            {
                const Matrix oldEntityMatrix = BuildRenderableEntityMatrix(position, rotation, currentScale);
                const Matrix oldWorldMatrix = MatrixMultiply(model->GetTransform(), oldEntityMatrix);
                const float scaleFactor = nextUniformScale / currentUniformScale;
                const Matrix scaleDelta = MatrixScale(scaleFactor, scaleFactor, scaleFactor);
                const Matrix desiredWorldMatrix =
                    MatrixMultiply(oldWorldMatrix, BuildPivotDeltaMatrix(PivotWorldPosition(entity), scaleDelta));
                applyWorldMatrix(entity, desiredWorldMatrix, position, rotation, nextScale);
            }
            else
            {
                sys->registry->get<sage::sgTransform>(entity).SetWorldScale(nextScale);
                updateEntityCollisionBounds(entity);
            }
        }
        else
        {
            sys->registry->get<sage::sgTransform>(entity).SetWorldScale(nextScale);
            updateEntityCollisionBounds(entity);
        }

        notify(entity);
    }

    void EditorTransformEditor::applyWorldMatrix(
        const entt::entity entity,
        const Matrix desiredWorldMatrix,
        const Vector3 position,
        const Vector3 rotation,
        const Vector3 scale)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        sys->registry->get<sage::sgTransform>(entity).SetWorldPos(position);
        sys->registry->get<sage::sgTransform>(entity).SetWorldRot(rotation);
        sys->registry->get<sage::sgTransform>(entity).SetWorldScale(scale);

        if (sys->registry->any_of<Renderable>(entity))
        {
            auto& renderable = sys->registry->get<Renderable>(entity);
            if (auto* model = renderable.GetModel(); model != nullptr)
            {
                const Matrix entityMatrix = BuildRenderableEntityMatrix(position, rotation, scale);
                const Matrix modelTransform = MatrixMultiply(desiredWorldMatrix, MatrixInvert(entityMatrix));
                model->SetTransform(modelTransform);
                renderable.initialTransform = modelTransform;
            }
        }

        updateEntityCollisionBounds(entity);
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
