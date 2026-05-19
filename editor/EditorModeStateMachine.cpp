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
        machine.scene.refreshOverlay();
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
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.scene.sys->UI().GetCellUnderCursor())
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
        const auto entity =
            machine.scene.pickingService->PickSceneEntity(GetMousePosition(), machine.scene.placementController->GridSurfaceEntity());
        if (!entity.has_value()) return false;

        SelectSceneEntity(machine, *entity);
        return true;
    }

    void EditorSelectState::ClearSceneEntitySelection(EditorModeStateMachine& machine)
    {
        machine.scene.selection->Clear();
        if (machine.IsEditMode())
        {
            machine.ChangeState(EditorSelectState{});
        }
        machine.scene.gui->HideDeleteConfirmation();
        machine.scene.refreshSceneWindows();
    }

    void EditorSelectState::SelectSceneEntity(EditorModeStateMachine& machine, const entt::entity entity)
    {
        if (!machine.scene.selection->Select(entity)) return;
        machine.ChangeState(EditorSelectState{});
        machine.scene.gui->HideDeleteConfirmation();
        machine.scene.refreshSceneWindows();
    }

    void EditorSelectState::RequestDeleteSelectedEntity(EditorModeStateMachine& machine)
    {
        if (!machine.scene.selection->HasSelection()) return;
        if (!machine.scene.selection->ActiveTransformEntity().has_value())
        {
            ClearSceneEntitySelection(machine);
            return;
        }

        machine.scene.gui->ShowDeleteConfirmation(machine.scene.describeSelectedSceneEntity());
    }

    void EditorSelectState::CancelDeleteSelectedEntity(EditorModeStateMachine& machine)
    {
        machine.scene.gui->HideDeleteConfirmation();
    }

    void EditorSelectState::ConfirmDeleteSelectedEntity(EditorModeStateMachine& machine)
    {
        const auto selectedEntity = machine.scene.selection->Current();
        if (!selectedEntity.has_value())
        {
            machine.scene.gui->HideDeleteConfirmation();
            return;
        }

        const auto entity = *selectedEntity;
        machine.scene.selection->Clear();
        machine.scene.gui->HideDeleteConfirmation();
        machine.scene.entityOperations->DeleteEntityAndChildren(entity);
        machine.scene.refreshSceneWindows();
        machine.scene.refreshOverlay();
    }

    void EditorSelectState::BeginEditSelectedTransform(EditorModeStateMachine& machine)
    {
        const auto selectedEntity = machine.scene.selection->ActiveTransformEntity();
        if (!selectedEntity.has_value())
        {
            ClearSceneEntitySelection(machine);
            return;
        }

        machine.scene.gui->HideDeleteConfirmation();
        machine.ChangeState(EditorEditState{.entity = *selectedEntity});
        machine.scene.refreshOverlay();
        machine.scene.refreshSceneWindows();
    }

    // ===== Place Mode ==============================================================

    void EditorPlaceState::OnEnter(EditorModeStateMachine& machine, entt::entity)
    {
        machine.scene.assetCatalog->Select(placeableIndex);
        machine.scene.resetPlacementTransform();
        machine.scene.refreshOverlay();
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
                machine.scene.adjustPlacementScale(-PLACEMENT_SCALE_STEP);
            }
            else
            {
                machine.scene.adjustPlacementRotation(-PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                machine.scene.adjustPlacementScale(PLACEMENT_SCALE_STEP);
            }
            else
            {
                machine.scene.adjustPlacementRotation(PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_P))
        {
            machine.scene.placeSelectedMesh();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.scene.sys->UI().GetCellUnderCursor())
        {
            if (!SelectSceneEntityUnderCursor(machine))
            {
                machine.scene.placeSelectedMesh();
            }
        }
    }

    void EditorPlaceState::Draw3D(const EditorModeStateMachine& machine, entt::entity) const
    {
        const auto& snappedPlacementPosition = machine.scene.placementController->SnappedPlacementPosition();
        if (!snappedPlacementPosition.has_value()) return;

        machine.scene.drawPlacementPreview();

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
        if (!machine.scene.sys->registry->valid(entity) ||
            !machine.scene.sys->registry->any_of<sgTransform>(entity))
        {
            machine.ChangeState(EditorSelectState{});
            return;
        }

        machine.scene.selection->Select(entity);
        machine.transformEditor.EnterEditMode(entity, *this);
        machine.scene.syncPlacementFromEntity(entity);
        machine.scene.refreshOverlay();
        machine.scene.refreshSceneWindows();
    }

    void EditorEditState::OnExit(EditorModeStateMachine& machine, entt::entity)
    {
        machine.transformEditor.ExitEditMode();
    }

    void EditorEditState::Update(EditorModeStateMachine& machine, entt::entity)
    {
        if (!machine.scene.sys->registry->valid(entity) ||
            !machine.scene.sys->registry->any_of<sgTransform>(entity))
        {
            EditorSelectState{}.ClearSceneEntitySelection(machine);
            return;
        }

        machine.scene.selection->Select(entity);
        machine.scene.syncPlacementFromEntity(entity);

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

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !machine.scene.sys->UI().GetCellUnderCursor() &&
            machine.scene.sys->registry->valid(entity) && machine.scene.sys->registry->any_of<sgTransform>(entity))
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
        const auto& snappedPlacementPosition = machine.scene.placementController->SnappedPlacementPosition();
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
        machine.scene.refreshOverlay();
        machine.scene.refreshSceneWindows();
    }

    bool EditorEditState::CancelEditSelectedTransform(EditorModeStateMachine& machine)
    {
        machine.transformEditor.RestoreSnapshot(*this);
        machine.ChangeState(EditorSelectState{});
        machine.scene.refreshOverlay();
        machine.scene.refreshSceneWindows();
        return true;
    }

    void EditorEditState::ToggleEditPivotMode(EditorModeStateMachine& machine)
    {
        if (!machine.scene.sys->registry->valid(entity) ||
            !machine.scene.sys->registry->any_of<sgTransform>(entity))
        {
            EditorSelectState{}.ClearSceneEntitySelection(machine);
            return;
        }

        machine.transformEditor.TogglePivotMode();
        machine.scene.syncPlacementFromEntity(entity);
        machine.scene.refreshOverlay();
        machine.scene.refreshSceneWindows();
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
