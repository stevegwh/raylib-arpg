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

    void EditorModeStateMachine::onEnter(EditorSelectState&, entt::entity)
    {
        scene.refreshOverlay();
    }

    void EditorModeStateMachine::onExit(EditorSelectState&, entt::entity)
    {
    }

    void EditorModeStateMachine::update(EditorSelectState&, entt::entity)
    {
        if (TextInput::AnyEditing()) return;

        if (IsKeyPressed(KEY_TAB))
        {
            scene.toggleEditSelectedTransform();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !scene.sys->UI().GetCellUnderCursor())
        {
            if (!scene.selectSceneEntityUnderCursor())
            {
                scene.clearSceneEntitySelection();
            }
        }
    }

    void EditorModeStateMachine::draw3D(const EditorSelectState&, entt::entity) const
    {
    }

    // ===== Place Mode ==============================================================

    void EditorModeStateMachine::onEnter(EditorPlaceState& state, entt::entity)
    {
        scene.assetCatalog->Select(state.placeableIndex);
        scene.resetPlacementTransform();
        scene.refreshOverlay();
    }

    void EditorModeStateMachine::onExit(EditorPlaceState&, entt::entity)
    {
    }

    void EditorModeStateMachine::update(EditorPlaceState&, entt::entity)
    {
        if (TextInput::AnyEditing()) return;

        if (IsKeyPressed(KEY_LEFT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                scene.adjustPlacementScale(-PLACEMENT_SCALE_STEP);
            }
            else
            {
                scene.adjustPlacementRotation(-PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                scene.adjustPlacementScale(PLACEMENT_SCALE_STEP);
            }
            else
            {
                scene.adjustPlacementRotation(PLACEMENT_ROTATION_STEP);
            }
        }
        if (IsKeyPressed(KEY_P))
        {
            scene.placeSelectedMesh();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !scene.sys->UI().GetCellUnderCursor())
        {
            if (!scene.selectSceneEntityUnderCursor())
            {
                scene.placeSelectedMesh();
            }
        }
    }

    void EditorModeStateMachine::draw3D(const EditorPlaceState&, entt::entity) const
    {
        const auto& snappedPlacementPosition = scene.placementController->SnappedPlacementPosition();
        if (!snappedPlacementPosition.has_value()) return;

        scene.drawPlacementPreview();

        const Vector3 marker = {
            snappedPlacementPosition->x,
            snappedPlacementPosition->y + PLACEMENT_MARKER_HEIGHT,
            snappedPlacementPosition->z};
        DrawCubeWires(marker, 1.0f, PLACEMENT_MARKER_HEIGHT, 1.0f, GOLD);
        DrawSphere(marker, 0.08f, GOLD);
    }

    // ===== Edit Mode ===============================================================

    void EditorModeStateMachine::onEnter(EditorEditState& state, entt::entity)
    {
        if (!scene.sys->registry->valid(state.entity) || !scene.sys->registry->any_of<sgTransform>(state.entity))
        {
            ChangeState(EditorSelectState{});
            return;
        }

        scene.selectedSceneEntity = state.entity;
        transformEditor.EnterEditMode(state.entity, state);
        scene.syncPlacementFromEntity(state.entity);
        scene.refreshOverlay();
        scene.refreshSceneWindows();
    }

    void EditorModeStateMachine::onExit(EditorEditState&, entt::entity)
    {
        transformEditor.ExitEditMode();
    }

    void EditorModeStateMachine::update(EditorEditState& state, entt::entity)
    {
        if (!scene.sys->registry->valid(state.entity) || !scene.sys->registry->any_of<sgTransform>(state.entity))
        {
            scene.clearSceneEntitySelection();
            return;
        }

        scene.selectedSceneEntity = state.entity;
        scene.syncPlacementFromEntity(state.entity);

        if (TextInput::AnyEditing()) return;

        if (IsKeyPressed(KEY_TAB))
        {
            scene.finishEditSelectedTransform();
            return;
        }

        if (transformEditor.IsGizmoDragging())
        {
            transformEditor.Update(state.entity);
            return;
        }

        if (IsKeyPressed(KEY_T))
        {
            transformEditor.SetMode(EditGizmo::Mode::Translate);
        }
        if (IsKeyPressed(KEY_R))
        {
            transformEditor.SetMode(EditGizmo::Mode::Rotate);
        }
        if (IsKeyPressed(KEY_Y))
        {
            transformEditor.SetMode(EditGizmo::Mode::Scale);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !scene.sys->UI().GetCellUnderCursor() &&
            scene.sys->registry->valid(state.entity) && scene.sys->registry->any_of<sgTransform>(state.entity))
        {
            if (transformEditor.TryStartDrag(state.entity, GetMousePosition())) return;
        }

        const bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        switch (transformEditor.Mode())
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
                transformEditor.AdjustPosition(state.entity, positionDelta);
            }
            break;
        }
        case EditGizmo::Mode::Rotate:
        {
            if (IsKeyPressedOrRepeated(KEY_LEFT))
            {
                transformEditor.AdjustRotationAxis(state.entity, EditGizmo::Axis::Y, -PLACEMENT_ROTATION_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_RIGHT))
            {
                transformEditor.AdjustRotationAxis(state.entity, EditGizmo::Axis::Y, PLACEMENT_ROTATION_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_UP))
            {
                transformEditor.AdjustRotationAxis(state.entity, EditGizmo::Axis::X, PLACEMENT_ROTATION_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_DOWN))
            {
                transformEditor.AdjustRotationAxis(state.entity, EditGizmo::Axis::X, -PLACEMENT_ROTATION_STEP);
            }
            break;
        }
        case EditGizmo::Mode::Scale:
        {
            if (IsKeyPressedOrRepeated(KEY_LEFT))
            {
                transformEditor.AdjustScale(state.entity, -PLACEMENT_SCALE_STEP);
            }
            if (IsKeyPressedOrRepeated(KEY_RIGHT))
            {
                transformEditor.AdjustScale(state.entity, PLACEMENT_SCALE_STEP);
            }
            break;
        }
        }

        if (IsKeyPressed(KEY_P))
        {
            scene.finishEditSelectedTransform();
        }
    }

    void EditorModeStateMachine::draw3D(const EditorEditState& state, entt::entity) const
    {
        const auto& snappedPlacementPosition = scene.placementController->SnappedPlacementPosition();
        if (!snappedPlacementPosition.has_value()) return;

        const Vector3 marker = {
            snappedPlacementPosition->x,
            snappedPlacementPosition->y + PLACEMENT_MARKER_HEIGHT,
            snappedPlacementPosition->z};
        DrawCubeWires(marker, 1.0f, PLACEMENT_MARKER_HEIGHT, 1.0f, ORANGE);
        DrawSphere(marker, 0.08f, ORANGE);

        transformEditor.Draw3D(state.entity);
    }

    // ===== Lifecycle ===============================================================

    void EditorModeStateMachine::Update()
    {
        auto& state = registry->get<EditorModeState>(stateEntity);
        std::visit([this](auto& current) { update(current, stateEntity); }, state.current);
    }

    void EditorModeStateMachine::Draw3D() const
    {
        const auto& state = registry->get<EditorModeState>(stateEntity);
        std::visit([this](const auto& current) { draw3D(current, stateEntity); }, state.current);
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
