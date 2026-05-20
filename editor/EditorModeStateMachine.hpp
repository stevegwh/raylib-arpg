#pragma once

#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"
#include "raylib.h"
#include "raymath.h"

#include <cstddef>
#include <optional>
#include <utility>
#include <variant>

namespace sage
{
    class EditorScene;
}

namespace sage::editor
{
    class EditorModeStateMachine;
    class EditorTransformEditor;

    struct EditorSelectState
    {
        void OnEnter(EditorModeStateMachine& machine, entt::entity stateEntity);
        void OnExit(EditorModeStateMachine& machine, entt::entity stateEntity);
        void Update(EditorModeStateMachine& machine, entt::entity stateEntity);
        void Draw3D(const EditorModeStateMachine& machine, entt::entity stateEntity) const;
        bool SelectSceneEntityUnderCursor(EditorModeStateMachine& machine);
        void ClearSceneEntitySelection(EditorModeStateMachine& machine);
        void SelectSceneEntity(EditorModeStateMachine& machine, entt::entity entity);
        void RequestDeleteSelectedEntity(EditorModeStateMachine& machine);
        void CancelDeleteSelectedEntity(EditorModeStateMachine& machine);
        void ConfirmDeleteSelectedEntity(EditorModeStateMachine& machine);
        void BeginEditSelectedTransform(EditorModeStateMachine& machine);
    };

    struct EditorPlaceState
    {
        std::size_t placeableIndex = 0;

        void OnEnter(EditorModeStateMachine& machine, entt::entity stateEntity);
        void OnExit(EditorModeStateMachine& machine, entt::entity stateEntity);
        void Update(EditorModeStateMachine& machine, entt::entity stateEntity);
        void Draw3D(const EditorModeStateMachine& machine, entt::entity stateEntity) const;
        bool SelectSceneEntityUnderCursor(EditorModeStateMachine& machine);
    };

    struct EditorEditState
    {
        entt::entity entity = entt::null;
        Vector3 originalPosition{};
        Vector3 originalRotation{};
        Vector3 originalScale{1.0f, 1.0f, 1.0f};
        Matrix originalRenderableTransform = MatrixIdentity();
        Matrix originalRenderableInitialTransform = MatrixIdentity();
        BoundingBox originalLocalBoundingBox{};
        BoundingBox originalWorldBoundingBox{};
        bool hadRenderable = false;
        bool hadCollideable = false;

        void OnEnter(EditorModeStateMachine& machine, entt::entity stateEntity);
        void OnExit(EditorModeStateMachine& machine, entt::entity stateEntity);
        void Update(EditorModeStateMachine& machine, entt::entity stateEntity);
        void Draw3D(const EditorModeStateMachine& machine, entt::entity stateEntity) const;
        void FinishEditSelectedTransform(EditorModeStateMachine& machine);
        [[nodiscard]] bool CancelEditSelectedTransform(EditorModeStateMachine& machine);
        void ToggleEditPivotMode(EditorModeStateMachine& machine);
    };

    struct EditorModeState
    {
        using Variant = std::variant<EditorSelectState, EditorPlaceState, EditorEditState>;

        Variant current = EditorSelectState{};

        void RemoveAllSubscriptions()
        {
        }
    };

    class EditorModeStateMachine final : public StateMachineBase<EditorModeStateMachine, EditorModeState>
    {
        using Base = StateMachineBase<EditorModeStateMachine, EditorModeState>;
        friend Base;
        friend struct EditorSelectState;
        friend struct EditorPlaceState;
        friend struct EditorEditState;

        EditorScene& scene;
        EditorTransformEditor& transformEditor;
        entt::entity stateEntity = entt::null;

        void refreshOverlay() const;
        void refreshSceneWindows() const;
        void resetPlacementTransform();
        void adjustPlacementRotation(float amount);
        void adjustPlacementScale(float amount);
        void syncPlacementFromEntity(entt::entity entity);
        [[nodiscard]] bool placeSelectedMesh();
        void drawPlacementPreview() const;
        [[nodiscard]] bool isMouseOverUiCell() const;
        [[nodiscard]] std::optional<entt::entity> pickSceneEntityUnderCursor() const;
        void clearSelection();
        [[nodiscard]] bool selectSelection(entt::entity entity);
        [[nodiscard]] bool hasSelection() const;
        [[nodiscard]] std::optional<entt::entity> activeTransformEntity() const;
        [[nodiscard]] std::optional<entt::entity> currentSelection() const;
        void hideDeleteConfirmation() const;
        void showDeleteConfirmationForSelection() const;
        void deleteEntityAndChildren(entt::entity entity) const;
        void selectPlaceableAsset(std::size_t index);
        [[nodiscard]] const std::optional<Vector3>& snappedPlacementPosition() const;
        [[nodiscard]] bool hasTransform(entt::entity entity) const;

        template <typename State>
        void onEnter(State& state, const entt::entity stateEntity)
        {
            state.OnEnter(*this, stateEntity);
        }

        template <typename State>
        void onExit(State& state, const entt::entity stateEntity)
        {
            state.OnExit(*this, stateEntity);
        }

      public:
        template <typename NewState>
        void ChangeState(NewState newState = {})
        {
            Base::ChangeState(stateEntity, std::move(newState));
        }

        void Update();
        void Draw3D() const;
        void RefreshPlacementTarget();
        void AdjustGridSurfaceY(float amount);
        void SelectPlaceable(std::size_t index);
        void SelectSceneEntity(entt::entity entity);
        void RequestDeleteSelectedEntity();
        void CancelDeleteSelectedEntity();
        void ConfirmDeleteSelectedEntity();
        bool HandleEscapePressed();
        void OnTransformApplied(entt::entity entity);

        [[nodiscard]] bool IsPlaceMode() const;
        [[nodiscard]] bool IsEditMode() const;
        [[nodiscard]] bool IsSelectMode() const;
        [[nodiscard]] EditorEditState* CurrentEditState();
        [[nodiscard]] const EditorEditState* CurrentEditState() const;

        EditorModeStateMachine(entt::registry* registry, EditorScene& scene, EditorTransformEditor& transformEditor);
    };
} // namespace sage::editor
