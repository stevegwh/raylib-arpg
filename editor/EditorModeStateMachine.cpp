#include "EditorModeStateMachine.hpp"

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

    void EditorSelectState::OnEnter(EditorModeStateMachine& machine, entt::entity)
    {
        machine.refreshOverlay();
    }

    void EditorSelectState::OnExit(EditorModeStateMachine&, entt::entity)
    {
    }

    void EditorSelectState::Update(EditorModeStateMachine& machine, entt::entity)
    {
        if (TextInput::AnyEditing()) return;

        if (IsKeyPressed(KEY_TAB))
        {
            BeginEditSelectedTransform(machine);
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.isMouseOverUiCell())
        {
            if (!SelectSceneEntityUnderCursor(machine))
            {
                ClearSceneEntitySelection(machine);
            }
        }
    }

    void EditorSelectState::Draw3D(const EditorModeStateMachine&, entt::entity) const
    {
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

    // ===== Place Mode ==============================================================

    void EditorPlaceState::OnEnter(EditorModeStateMachine& machine, entt::entity)
    {
        machine.selectPlaceableAsset(placeableIndex);
        machine.resetPlacementTransform();
        machine.refreshOverlay();
    }

    void EditorPlaceState::OnExit(EditorModeStateMachine&, entt::entity)
    {
    }

    void EditorPlaceState::Update(EditorModeStateMachine& machine, entt::entity)
    {
        if (TextInput::AnyEditing()) return;

        if (IsKeyPressed(KEY_LEFT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                machine.adjustPlacementScale(-PLACEMENT_SCALE_STEP);
            }
            else
            {
                machine.adjustPlacementRotation(-PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                machine.adjustPlacementScale(PLACEMENT_SCALE_STEP);
            }
            else
            {
                machine.adjustPlacementRotation(PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_P))
        {
            (void)machine.placeSelectedMesh();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.isMouseOverUiCell())
        {
            if (!machine.placeSelectedMesh())
            {
                SelectSceneEntityUnderCursor(machine);
            }
        }
    }

    void EditorPlaceState::Draw3D(const EditorModeStateMachine& machine, entt::entity) const
    {
        const auto& snappedPlacementPosition = machine.snappedPlacementPosition();
        if (!snappedPlacementPosition.has_value()) return;

        machine.drawPlacementPreview();

        const Vector3 marker = {
            snappedPlacementPosition->x,
            snappedPlacementPosition->y + PLACEMENT_MARKER_HEIGHT,
            snappedPlacementPosition->z};
        DrawCubeWires(marker, 1.0f, PLACEMENT_MARKER_HEIGHT, 1.0f, GOLD);
        DrawSphere(marker, 0.08f, GOLD);
    }

    bool EditorPlaceState::SelectSceneEntityUnderCursor(EditorModeStateMachine& machine)
    {
        return EditorSelectState{}.SelectSceneEntityUnderCursor(machine);
    }

    // ===== Edit Mode ===============================================================

    void EditorEditState::OnEnter(EditorModeStateMachine& machine, entt::entity)
    {
        if (!machine.hasTransform(entity))
        {
            machine.ChangeState(EditorSelectState{});
            return;
        }

        (void)machine.selectSelection(entity);
        machine.transformEditor.EnterEditMode(entity, *this);
        machine.syncPlacementFromEntity(entity);
        machine.refreshOverlay();
        machine.refreshSceneWindows();
    }

    void EditorEditState::OnExit(EditorModeStateMachine& machine, entt::entity)
    {
        machine.transformEditor.ExitEditMode();
    }

    void EditorEditState::Update(EditorModeStateMachine& machine, entt::entity)
    {
        if (!machine.hasTransform(entity))
        {
            EditorSelectState{}.ClearSceneEntitySelection(machine);
            return;
        }

        (void)machine.selectSelection(entity);
        machine.syncPlacementFromEntity(entity);

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

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.isMouseOverUiCell() && machine.hasTransform(entity))
        {
            if (machine.transformEditor.TryStartDrag(entity, GetMousePosition())) return;
        }

        const bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        switch (machine.transformEditor.Mode())
        {
        case EditGizmo::Mode::Translate:
        {
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
        case EditGizmo::Mode::Rotate:
        {
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
        case EditGizmo::Mode::Scale:
        {
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

    void EditorEditState::Draw3D(const EditorModeStateMachine& machine, entt::entity) const
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
            EditorSelectState{}.ClearSceneEntitySelection(machine);
            return;
        }

        machine.transformEditor.TogglePivotMode();
        machine.syncPlacementFromEntity(entity);
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

    void EditorModeStateMachine::resetPlacementTransform()
    {
        scene.placementController->ResetTransform();
    }

    void EditorModeStateMachine::AdjustGridSurfaceY(const float amount)
    {
        scene.placementController->AdjustGridSurfaceY(amount);
        refreshOverlay();
    }

    void EditorModeStateMachine::adjustPlacementRotation(const float amount)
    {
        scene.placementController->AdjustRotation(amount);
        refreshOverlay();
    }

    void EditorModeStateMachine::adjustPlacementScale(const float amount)
    {
        scene.placementController->AdjustScale(amount);
        refreshOverlay();
    }

    void EditorModeStateMachine::syncPlacementFromEntity(const entt::entity entity)
    {
        scene.placementController->SyncFromEntity(entity);
    }

    bool EditorModeStateMachine::placeSelectedMesh()
    {
        if (!IsPlaceMode()) return false;

        const auto entity = scene.placementController->PlaceSelectedMesh();
        if (!entity.has_value()) return false;

        scene.selection->Select(*entity);
        refreshSceneWindows();
        scene.gui->FocusHierarchyOnEntity(*entity);
        ChangeState(EditorSelectState{});
        refreshOverlay();
        return true;
    }

    void EditorModeStateMachine::drawPlacementPreview() const
    {
        scene.placementController->DrawPreview();
    }

    bool EditorModeStateMachine::isMouseOverUiCell() const
    {
        return scene.sys->UI().GetCellUnderCursor() != nullptr;
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

    void EditorModeStateMachine::SelectPlaceable(const std::size_t index)
    {
        if (index >= scene.assetCatalog->Size()) return;
        ChangeState(EditorPlaceState{.placeableIndex = index});
    }

    void EditorModeStateMachine::SelectSceneEntity(const entt::entity entity)
    {
        EditorSelectState{}.SelectSceneEntity(*this, entity);
    }

    void EditorModeStateMachine::RequestDeleteSelectedEntity()
    {
        EditorSelectState{}.RequestDeleteSelectedEntity(*this);
    }

    void EditorModeStateMachine::CancelDeleteSelectedEntity()
    {
        EditorSelectState{}.CancelDeleteSelectedEntity(*this);
    }

    void EditorModeStateMachine::ConfirmDeleteSelectedEntity()
    {
        EditorSelectState{}.ConfirmDeleteSelectedEntity(*this);
    }

    bool EditorModeStateMachine::HandleEscapePressed()
    {
        if (IsPlaceMode())
        {
            resetPlacementTransform();
            ChangeState(EditorSelectState{});
            refreshOverlay();
            refreshSceneWindows();
            return true;
        }

        auto* editState = CurrentEditState();
        return editState != nullptr && editState->CancelEditSelectedTransform(*this);
    }

    void EditorModeStateMachine::OnTransformApplied(const entt::entity entity)
    {
        if (IsEditMode())
        {
            syncPlacementFromEntity(entity);
        }
        refreshOverlay();
        refreshSceneWindows();
    }

    // ===== Lifecycle ===============================================================

    void EditorModeStateMachine::Update()
    {
        auto& state = registry->get<EditorModeState>(stateEntity);
        std::visit([this](auto& current) { current.Update(*this, stateEntity); }, state.current);
    }

    void EditorModeStateMachine::Draw3D() const
    {
        const auto& state = registry->get<EditorModeState>(stateEntity);
        std::visit([this](const auto& current) { current.Draw3D(*this, stateEntity); }, state.current);
    }

    bool EditorModeStateMachine::IsPlaceMode() const
    {
        const auto& state = registry->get<EditorModeState>(stateEntity);
        return std::holds_alternative<EditorPlaceState>(state.current);
    }

    bool EditorModeStateMachine::IsEditMode() const
    {
        const auto& state = registry->get<EditorModeState>(stateEntity);
        return std::holds_alternative<EditorEditState>(state.current);
    }

    bool EditorModeStateMachine::IsSelectMode() const
    {
        const auto& state = registry->get<EditorModeState>(stateEntity);
        return std::holds_alternative<EditorSelectState>(state.current);
    }

    EditorEditState* EditorModeStateMachine::CurrentEditState()
    {
        auto& state = registry->get<EditorModeState>(stateEntity);
        return std::get_if<EditorEditState>(&state.current);
    }

    const EditorEditState* EditorModeStateMachine::CurrentEditState() const
    {
        const auto& state = registry->get<EditorModeState>(stateEntity);
        return std::get_if<EditorEditState>(&state.current);
    }

    EditorModeStateMachine::EditorModeStateMachine(
        entt::registry* registry,
        EditorScene& scene,
        EditorTransformEditor& transformEditor)
        : Base(registry), scene(scene), transformEditor(transformEditor), stateEntity(registry->create())
    {
        registry->emplace<EditorModeState>(stateEntity);
    }
} // namespace sage::editor
