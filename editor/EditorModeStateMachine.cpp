#include "EditorModeStateMachine.hpp"

#include "EditorGui.hpp"
#include "EditorScene.hpp"
#include "EditorTransformEditor.hpp"
#include "engine/Camera.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/Settings.hpp"
#include "engine/UserInput.hpp"

#include "raylib.h"

#include <algorithm>

namespace sage::editor
{
    namespace
    {
        constexpr float PLACEMENT_MARKER_HEIGHT = 0.16f;
        constexpr float EDIT_TRANSLATION_STEP = 0.25f;
        constexpr float PLACEMENT_ROTATION_STEP = 15.0f;
        constexpr float PLACEMENT_SCALE_STEP = 0.1f;

        bool IsKeyPressedOrRepeated(const int key)
        {
            return IsKeyPressed(key) || IsKeyPressedRepeat(key);
        }
    } // namespace

    // ===== Select Mode =============================================================

    void EditorSelectState::OnEnter(EditorModeStateMachine& machine)
    {
        machine.refreshOverlay();
    }

    void EditorSelectState::OnExit(EditorModeStateMachine&)
    {
    }

    void EditorSelectState::Update(EditorModeStateMachine& machine)
    {
        HandleDeleteConfirmationInput(machine);

        if (TextInput::AnyEditing()) return;

        HandleKeyboardInput(machine);
        HandleMouseInput(machine);
    }

    void EditorSelectState::Draw3D(const EditorModeStateMachine&) const
    {
    }

    void EditorSelectState::HandleDeleteConfirmationInput(EditorModeStateMachine& machine)
    {
        const auto action = machine.consumeDeleteConfirmationAction();
        if (action == EditorGui::DeleteConfirmationAction::None) return;

        if (action == EditorGui::DeleteConfirmationAction::Confirm)
        {
            ConfirmDeleteSelectedEntity(machine);
        }
        else
        {
            CancelDeleteSelectedEntity(machine);
        }
    }

    void EditorSelectState::HandleKeyboardInput(EditorModeStateMachine& machine)
    {
        if (IsKeyPressed(KEY_DELETE) && !machine.isDeleteConfirmationVisible())
        {
            RequestDeleteSelectedEntity(machine);
        }

        if (IsKeyPressed(KEY_F) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)))
        {
            machine.focusSelectedObjectInHierarchy();
        }
        else if (IsKeyPressed(KEY_F))
        {
            machine.focusSelectedObject();
        }

        if (IsKeyPressed(KEY_TAB))
        {
            BeginEditSelectedTransform(machine);
        }
    }

    void EditorSelectState::HandleMouseInput(EditorModeStateMachine& machine)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.isMouseOverUiCell())
        {
            if (!SelectSceneEntityUnderCursor(machine))
            {
                ClearSceneEntitySelection(machine);
            }
        }
    }

    bool EditorSelectState::SelectSceneEntityUnderCursor(EditorModeStateMachine& machine)
    {
        const auto entity = machine.pickSceneEntityUnderCursor();
        if (!entity.has_value()) return false;

        SelectSceneEntity(machine, *entity);
        return true;
    }

    void EditorSelectState::ClearSceneEntitySelection(EditorModeStateMachine& machine)
    {
        machine.clearSelection();
        if (machine.IsEditMode())
        {
            machine.ChangeState(EditorSelectState{});
        }
        machine.hideDeleteConfirmation();
        machine.refreshSceneWindows();
    }

    void EditorSelectState::SelectSceneEntity(EditorModeStateMachine& machine, const entt::entity entity)
    {
        if (!machine.selectSelection(entity)) return;
        machine.ChangeState(EditorSelectState{});
        machine.hideDeleteConfirmation();
        machine.refreshSceneWindows();
    }

    void EditorSelectState::RequestDeleteSelectedEntity(EditorModeStateMachine& machine)
    {
        if (!machine.hasSelection()) return;
        if (!machine.activeTransformEntity().has_value())
        {
            ClearSceneEntitySelection(machine);
            return;
        }

        machine.showDeleteConfirmationForSelection();
    }

    void EditorSelectState::CancelDeleteSelectedEntity(EditorModeStateMachine& machine)
    {
        machine.hideDeleteConfirmation();
    }

    void EditorSelectState::ConfirmDeleteSelectedEntity(EditorModeStateMachine& machine)
    {
        const auto selectedEntity = machine.currentSelection();
        if (!selectedEntity.has_value())
        {
            machine.hideDeleteConfirmation();
            return;
        }

        const auto entity = *selectedEntity;
        machine.clearSelection();
        machine.hideDeleteConfirmation();
        machine.deleteEntityAndChildren(entity);
        machine.refreshSceneWindows();
        machine.refreshOverlay();
    }

    void EditorSelectState::BeginEditSelectedTransform(EditorModeStateMachine& machine)
    {
        const auto selectedEntity = machine.activeTransformEntity();
        if (!selectedEntity.has_value())
        {
            ClearSceneEntitySelection(machine);
            return;
        }

        machine.hideDeleteConfirmation();
        machine.ChangeState(EditorEditState{.entity = *selectedEntity});
        machine.refreshOverlay();
        machine.refreshSceneWindows();
    }

    bool EditorSelectState::HandleEscape(EditorModeStateMachine&)
    {
        return false;
    }

    void EditorSelectState::OnTransformApplied(EditorModeStateMachine&, entt::entity)
    {
    }

    // ===== Place Mode ==============================================================

    void EditorPlaceState::OnEnter(EditorModeStateMachine& machine)
    {
        machine.selectPlaceableAsset(placeableIndex);
        ResetPlacementTransform(machine);
        machine.refreshOverlay();
    }

    void EditorPlaceState::OnExit(EditorModeStateMachine&)
    {
    }

    void EditorPlaceState::Update(EditorModeStateMachine& machine)
    {
        if (TextInput::AnyEditing()) return;

        if (IsKeyPressed(KEY_LEFT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                AdjustPlacementScale(machine, -PLACEMENT_SCALE_STEP);
            }
            else
            {
                AdjustPlacementRotation(machine, -PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                AdjustPlacementScale(machine, PLACEMENT_SCALE_STEP);
            }
            else
            {
                AdjustPlacementRotation(machine, PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_P))
        {
            (void)PlaceSelectedMesh(machine);
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.isMouseOverUiCell())
        {
            if (!PlaceSelectedMesh(machine))
            {
                SelectSceneEntityUnderCursor(machine);
            }
        }
    }

    void EditorPlaceState::Draw3D(const EditorModeStateMachine& machine) const
    {
        const auto& snappedPlacementPosition = machine.snappedPlacementPosition();
        if (!snappedPlacementPosition.has_value()) return;

        DrawPlacementPreview(machine);

        const Vector3 marker = {
            snappedPlacementPosition->x,
            snappedPlacementPosition->y + PLACEMENT_MARKER_HEIGHT,
            snappedPlacementPosition->z};
        DrawCubeWires(marker, 1.0f, PLACEMENT_MARKER_HEIGHT, 1.0f, GOLD);
        DrawSphere(marker, 0.08f, GOLD);
    }

    bool EditorPlaceState::SelectSceneEntityUnderCursor(EditorModeStateMachine& machine)
    {
        const auto entity = machine.pickSceneEntityUnderCursor();
        if (!entity.has_value()) return false;

        machine.SelectSceneEntity(*entity);
        return true;
    }

    void EditorPlaceState::ResetPlacementTransform(EditorModeStateMachine& machine)
    {
        machine.placement().ResetTransform();
    }

    void EditorPlaceState::AdjustPlacementRotation(EditorModeStateMachine& machine, const float amount)
    {
        machine.placement().AdjustRotation(amount);
        machine.refreshOverlay();
    }

    void EditorPlaceState::AdjustPlacementScale(EditorModeStateMachine& machine, const float amount)
    {
        machine.placement().AdjustScale(amount);
        machine.refreshOverlay();
    }

    bool EditorPlaceState::PlaceSelectedMesh(EditorModeStateMachine& machine)
    {
        const auto entity = machine.placement().PlaceSelectedMesh();
        if (!entity.has_value()) return false;

        (void)machine.selectSelection(*entity);
        machine.refreshSceneWindows();
        machine.focusHierarchyOnEntity(*entity);
        machine.ChangeState(EditorSelectState{});
        machine.refreshOverlay();
        return true;
    }

    void EditorPlaceState::DrawPlacementPreview(const EditorModeStateMachine& machine) const
    {
        machine.placement().DrawPreview();
    }

    bool EditorPlaceState::HandleEscape(EditorModeStateMachine& machine)
    {
        ResetPlacementTransform(machine);
        machine.ChangeState(EditorSelectState{});
        machine.refreshOverlay();
        machine.refreshSceneWindows();
        return true;
    }

    void EditorPlaceState::OnTransformApplied(EditorModeStateMachine&, entt::entity)
    {
    }

    // ===== Edit Mode ===============================================================

    void EditorEditState::OnEnter(EditorModeStateMachine& machine)
    {
        if (!machine.hasTransform(entity))
        {
            machine.ChangeState(EditorSelectState{});
            return;
        }

        (void)machine.selectSelection(entity);
        machine.transformEditor.EnterEditMode(entity, *this);
        SyncPlacementFromEntity(machine, entity);
        machine.refreshOverlay();
        machine.refreshSceneWindows();
    }

    void EditorEditState::OnExit(EditorModeStateMachine& machine)
    {
        machine.transformEditor.ExitEditMode();
    }

    void EditorEditState::Update(EditorModeStateMachine& machine)
    {
        if (!machine.hasTransform(entity))
        {
            ClearSceneEntitySelection(machine);
            return;
        }

        (void)machine.selectSelection(entity);
        SyncPlacementFromEntity(machine, entity);

        if (TextInput::AnyEditing()) return;

        if (IsKeyPressed(KEY_TAB))
        {
            FinishEditSelectedTransform(machine);
            return;
        }

        if (machine.transformEditor.IsGizmoDragging())
        {
            machine.transformEditor.Update(entity);
            return;
        }

        if (IsKeyPressed(KEY_T))
        {
            machine.transformEditor.SetMode(EditGizmo::Mode::Translate);
        }
        if (IsKeyPressed(KEY_R))
        {
            machine.transformEditor.SetMode(EditGizmo::Mode::Rotate);
        }
        if (IsKeyPressed(KEY_Y))
        {
            machine.transformEditor.SetMode(EditGizmo::Mode::Scale);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.isMouseOverUiCell() &&
            machine.hasTransform(entity))
        {
            if (machine.transformEditor.TryStartDrag(entity, GetMousePosition())) return;
        }

        const bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        switch (machine.transformEditor.Mode())
        {
        case EditGizmo::Mode::Translate: {
            Vector3 positionDelta{};
            if (IsKeyPressedOrRepeated(KEY_LEFT))
            {
                positionDelta.x -= EDIT_TRANSLATION_STEP;
            }
            if (IsKeyPressedOrRepeated(KEY_RIGHT))
            {
                positionDelta.x += EDIT_TRANSLATION_STEP;
            }
            if (shiftDown)
            {
                if (IsKeyPressedOrRepeated(KEY_UP))
                {
                    positionDelta.y += EDIT_TRANSLATION_STEP;
                }
                if (IsKeyPressedOrRepeated(KEY_DOWN))
                {
                    positionDelta.y -= EDIT_TRANSLATION_STEP;
                }
            }
            else
            {
                if (IsKeyPressedOrRepeated(KEY_UP))
                {
                    positionDelta.z += EDIT_TRANSLATION_STEP;
                }
                if (IsKeyPressedOrRepeated(KEY_DOWN))
                {
                    positionDelta.z -= EDIT_TRANSLATION_STEP;
                }
            }
            if (positionDelta.x != 0.0f || positionDelta.y != 0.0f || positionDelta.z != 0.0f)
            {
                machine.transformEditor.AdjustPosition(entity, positionDelta);
            }
            break;
        }
        case EditGizmo::Mode::Rotate: {
            if (IsKeyPressedOrRepeated(KEY_LEFT))
            {
                machine.transformEditor.AdjustRotationAxis(entity, EditGizmo::Axis::Y, -PLACEMENT_ROTATION_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_RIGHT))
            {
                machine.transformEditor.AdjustRotationAxis(entity, EditGizmo::Axis::Y, PLACEMENT_ROTATION_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_UP))
            {
                machine.transformEditor.AdjustRotationAxis(entity, EditGizmo::Axis::X, PLACEMENT_ROTATION_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_DOWN))
            {
                machine.transformEditor.AdjustRotationAxis(entity, EditGizmo::Axis::X, -PLACEMENT_ROTATION_STEP);
            }
            break;
        }
        case EditGizmo::Mode::Scale: {
            if (IsKeyPressedOrRepeated(KEY_LEFT))
            {
                machine.transformEditor.AdjustScale(entity, -PLACEMENT_SCALE_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_RIGHT))
            {
                machine.transformEditor.AdjustScale(entity, PLACEMENT_SCALE_STEP);
            }
            break;
        }
        }

        if (IsKeyPressed(KEY_P))
        {
            FinishEditSelectedTransform(machine);
        }
    }

    void EditorEditState::Draw3D(const EditorModeStateMachine& machine) const
    {
        const auto& snappedPlacementPosition = machine.snappedPlacementPosition();
        if (!snappedPlacementPosition.has_value()) return;

        const Vector3 marker = {
            snappedPlacementPosition->x,
            snappedPlacementPosition->y + PLACEMENT_MARKER_HEIGHT,
            snappedPlacementPosition->z};
        DrawCubeWires(marker, 1.0f, PLACEMENT_MARKER_HEIGHT, 1.0f, ORANGE);
        DrawSphere(marker, 0.08f, ORANGE);

        machine.transformEditor.Draw3D(entity);
    }

    void EditorEditState::FinishEditSelectedTransform(EditorModeStateMachine& machine)
    {
        machine.ChangeState(EditorSelectState{});
        machine.refreshOverlay();
        machine.refreshSceneWindows();
    }

    bool EditorEditState::CancelEditSelectedTransform(EditorModeStateMachine& machine)
    {
        machine.transformEditor.RestoreSnapshot(*this);
        machine.ChangeState(EditorSelectState{});
        machine.refreshOverlay();
        machine.refreshSceneWindows();
        return true;
    }

    void EditorEditState::ToggleEditPivotMode(EditorModeStateMachine& machine)
    {
        if (!machine.hasTransform(entity))
        {
            ClearSceneEntitySelection(machine);
            return;
        }

        machine.transformEditor.TogglePivotMode();
        SyncPlacementFromEntity(machine, entity);
        machine.refreshOverlay();
        machine.refreshSceneWindows();
    }

    void EditorEditState::ClearSceneEntitySelection(EditorModeStateMachine& machine)
    {
        machine.clearSelection();
        machine.ChangeState(EditorSelectState{});
        machine.hideDeleteConfirmation();
        machine.refreshSceneWindows();
    }

    void EditorEditState::SyncPlacementFromEntity(EditorModeStateMachine& machine, const entt::entity entity)
    {
        machine.placement().SyncFromEntity(entity);
    }

    bool EditorEditState::HandleEscape(EditorModeStateMachine& machine)
    {
        return CancelEditSelectedTransform(machine);
    }

    void EditorEditState::OnTransformApplied(EditorModeStateMachine& machine, const entt::entity entity)
    {
        SyncPlacementFromEntity(machine, entity);
        machine.refreshOverlay();
        machine.refreshSceneWindows();
    }

    // ===== Controller actions =========================================================

    void EditorModeStateMachine::refreshOverlay() const
    {
        scene.refreshOverlay();
    }

    void EditorModeStateMachine::refreshSceneWindows() const
    {
        scene.refreshSceneWindows();
    }

    void EditorModeStateMachine::RefreshPlacementTarget()
    {
        scene.placementController->RefreshTarget();
    }

    void EditorModeStateMachine::AdjustGridSurfaceY(const float amount)
    {
        scene.placementController->AdjustGridSurfaceY(amount);
        refreshOverlay();
    }

    bool EditorModeStateMachine::isMouseOverUiCell() const
    {
        return scene.sys->UI().GetCellUnderCursor() != nullptr;
    }

    bool EditorModeStateMachine::isDeleteConfirmationVisible() const
    {
        return scene.gui->IsDeleteConfirmationVisible();
    }

    EditorGui::DeleteConfirmationAction EditorModeStateMachine::consumeDeleteConfirmationAction()
    {
        return scene.gui->ConsumeDeleteConfirmationAction();
    }

    std::optional<entt::entity> EditorModeStateMachine::pickSceneEntityUnderCursor() const
    {
        return scene.pickingService->PickSceneEntity(
            GetMousePosition(), scene.placementController->GridSurfaceEntity());
    }

    void EditorModeStateMachine::clearSelection()
    {
        scene.selection->Clear();
    }

    bool EditorModeStateMachine::selectSelection(const entt::entity entity)
    {
        return scene.selection->Select(entity);
    }

    bool EditorModeStateMachine::hasSelection() const
    {
        return scene.selection->HasSelection();
    }

    std::optional<entt::entity> EditorModeStateMachine::activeTransformEntity() const
    {
        return scene.selection->ActiveTransformEntity();
    }

    std::optional<entt::entity> EditorModeStateMachine::currentSelection() const
    {
        return scene.selection->Current();
    }

    void EditorModeStateMachine::hideDeleteConfirmation() const
    {
        scene.gui->HideDeleteConfirmation();
    }

    void EditorModeStateMachine::showDeleteConfirmationForSelection() const
    {
        scene.gui->ShowDeleteConfirmation(scene.describeSelectedSceneEntity());
    }

    void EditorModeStateMachine::deleteEntityAndChildren(const entt::entity entity) const
    {
        scene.entityOperations->DeleteEntityAndChildren(entity);
    }

    void EditorModeStateMachine::focusHierarchyOnEntity(const entt::entity entity) const
    {
        scene.gui->FocusHierarchyOnEntity(entity);
    }

    void EditorModeStateMachine::focusSelectedObject() const
    {
        scene.focusSelectedObject();
    }

    void EditorModeStateMachine::focusSelectedObjectInHierarchy() const
    {
        scene.focusSelectedObjectInHierarchy();
    }

    bool EditorModeStateMachine::canSelectPlaceable(const std::size_t index) const
    {
        return index < scene.assetCatalog->Size();
    }

    void EditorModeStateMachine::selectPlaceableAsset(const std::size_t index)
    {
        scene.assetCatalog->Select(index);
    }

    const std::optional<Vector3>& EditorModeStateMachine::snappedPlacementPosition() const
    {
        return scene.placementController->SnappedPlacementPosition();
    }

    bool EditorModeStateMachine::hasTransform(const entt::entity entity) const
    {
        return scene.sys->registry->valid(entity) && scene.sys->registry->any_of<sgTransform>(entity);
    }

    EditorPlacementController& EditorModeStateMachine::placement()
    {
        return *scene.placementController;
    }

    const EditorPlacementController& EditorModeStateMachine::placement() const
    {
        return *scene.placementController;
    }

    void EditorModeStateMachine::SelectPlaceable(const std::size_t index)
    {
        if (!canSelectPlaceable(index)) return;
        ChangeState(EditorPlaceState{.placeableIndex = index});
    }

    void EditorModeStateMachine::SelectSceneEntity(const entt::entity entity)
    {
        ChangeState(EditorSelectState{});
        std::get<EditorSelectState>(currentState).SelectSceneEntity(*this, entity);
    }

    bool EditorModeStateMachine::HandleEscapePressed()
    {
        return std::visit([this](auto& current) { return current.HandleEscape(*this); }, currentState);
    }

    void EditorModeStateMachine::OnTransformApplied(const entt::entity entity)
    {
        std::visit([this, entity](auto& current) { current.OnTransformApplied(*this, entity); }, currentState);
    }

    // ===== Lifecycle ===============================================================

    void EditorModeStateMachine::Update()
    {
        std::visit([this](auto& current) { current.Update(*this); }, currentState);
    }

    void EditorModeStateMachine::Draw3D() const
    {
        std::visit([this](const auto& current) { current.Draw3D(*this); }, currentState);
    }

    bool EditorModeStateMachine::IsPlaceMode() const
    {
        return std::holds_alternative<EditorPlaceState>(currentState);
    }

    bool EditorModeStateMachine::IsEditMode() const
    {
        return std::holds_alternative<EditorEditState>(currentState);
    }

    EditorEditState* EditorModeStateMachine::CurrentEditState()
    {
        return std::get_if<EditorEditState>(&currentState);
    }

    const EditorEditState* EditorModeStateMachine::CurrentEditState() const
    {
        return std::get_if<EditorEditState>(&currentState);
    }

    EditorModeStateMachine::EditorModeStateMachine(EditorScene& scene, EditorTransformEditor& transformEditor)
        : scene(scene), transformEditor(transformEditor)
    {
    }
} // namespace sage::editor
