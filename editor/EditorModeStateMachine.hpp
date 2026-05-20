#pragma once

#include "EditorGui.hpp"

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
    class EditorPlacementController;
    class EditorTransformEditor;

    struct EditorSelectState
    {
        void OnEnter(EditorModeStateMachine& machine);
        void OnExit(EditorModeStateMachine& machine);
        void Update(EditorModeStateMachine& machine);
        void Draw3D(const EditorModeStateMachine& machine) const;
        void HandleDeleteConfirmationInput(EditorModeStateMachine& machine);
        void HandleKeyboardInput(EditorModeStateMachine& machine);
        void HandleMouseInput(EditorModeStateMachine& machine);
        bool SelectSceneEntityUnderCursor(EditorModeStateMachine& machine);
        void ClearSceneEntitySelection(EditorModeStateMachine& machine);
        void SelectSceneEntity(EditorModeStateMachine& machine, entt::entity entity);
        void RequestDeleteSelectedEntity(EditorModeStateMachine& machine);
        void CancelDeleteSelectedEntity(EditorModeStateMachine& machine);
        void ConfirmDeleteSelectedEntity(EditorModeStateMachine& machine);
        void BeginEditSelectedTransform(EditorModeStateMachine& machine);
        [[nodiscard]] bool HandleEscape(EditorModeStateMachine& machine);
        void OnTransformApplied(EditorModeStateMachine& machine, entt::entity entity);
    };

    struct EditorPlaceState
    {
        std::size_t placeableIndex = 0;

        void OnEnter(EditorModeStateMachine& machine);
        void OnExit(EditorModeStateMachine& machine);
        void Update(EditorModeStateMachine& machine);
        void Draw3D(const EditorModeStateMachine& machine) const;
        bool SelectSceneEntityUnderCursor(EditorModeStateMachine& machine);
        void ResetPlacementTransform(EditorModeStateMachine& machine);
        void AdjustPlacementRotation(EditorModeStateMachine& machine, float amount);
        void AdjustPlacementScale(EditorModeStateMachine& machine, float amount);
        [[nodiscard]] bool PlaceSelectedMesh(EditorModeStateMachine& machine);
        void DrawPlacementPreview(const EditorModeStateMachine& machine) const;
        [[nodiscard]] bool HandleEscape(EditorModeStateMachine& machine);
        void OnTransformApplied(EditorModeStateMachine& machine, entt::entity entity);
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

        void OnEnter(EditorModeStateMachine& machine);
        void OnExit(EditorModeStateMachine& machine);
        void Update(EditorModeStateMachine& machine);
        void Draw3D(const EditorModeStateMachine& machine) const;
        void FinishEditSelectedTransform(EditorModeStateMachine& machine);
        [[nodiscard]] bool CancelEditSelectedTransform(EditorModeStateMachine& machine);
        void ToggleEditPivotMode(EditorModeStateMachine& machine);
        void ClearSceneEntitySelection(EditorModeStateMachine& machine);
        void SyncPlacementFromEntity(EditorModeStateMachine& machine, entt::entity entity);
        [[nodiscard]] bool HandleEscape(EditorModeStateMachine& machine);
        void OnTransformApplied(EditorModeStateMachine& machine, entt::entity entity);
    };

    class EditorModeStateMachine final
    {
        using State = std::variant<EditorSelectState, EditorPlaceState, EditorEditState>;

        friend struct EditorSelectState;
        friend struct EditorPlaceState;
        friend struct EditorEditState;

        EditorScene& scene;
        EditorTransformEditor& transformEditor;
        State currentState = EditorSelectState{};

        void refreshOverlay() const;
        void refreshSceneWindows() const;
        [[nodiscard]] bool isMouseOverUiCell() const;
        [[nodiscard]] bool isDeleteConfirmationVisible() const;
        [[nodiscard]] EditorGui::DeleteConfirmationAction consumeDeleteConfirmationAction();
        [[nodiscard]] std::optional<entt::entity> pickSceneEntityUnderCursor() const;
        void clearSelection();
        [[nodiscard]] bool selectSelection(entt::entity entity);
        [[nodiscard]] bool hasSelection() const;
        [[nodiscard]] std::optional<entt::entity> activeTransformEntity() const;
        [[nodiscard]] std::optional<entt::entity> currentSelection() const;
        void hideDeleteConfirmation() const;
        void showDeleteConfirmationForSelection() const;
        void deleteEntityAndChildren(entt::entity entity) const;
        void focusHierarchyOnEntity(entt::entity entity) const;
        void focusSelectedObject() const;
        void focusSelectedObjectInHierarchy() const;
        [[nodiscard]] bool canSelectPlaceable(std::size_t index) const;
        void selectPlaceableAsset(std::size_t index);
        [[nodiscard]] const std::optional<Vector3>& snappedPlacementPosition() const;
        [[nodiscard]] bool hasTransform(entt::entity entity) const;
        [[nodiscard]] EditorPlacementController& placement();
        [[nodiscard]] const EditorPlacementController& placement() const;

      public:
        template <typename NewState>
        void ChangeState(NewState newState = {})
        {
            std::visit([this](auto& current) { current.OnExit(*this); }, currentState);
            currentState = std::move(newState);
            std::get<NewState>(currentState).OnEnter(*this);
        }

        void Update();
        void Draw3D() const;
        void RefreshPlacementTarget();
        void AdjustGridSurfaceY(float amount);
        void SelectPlaceable(std::size_t index);
        void SelectSceneEntity(entt::entity entity);
        bool HandleEscapePressed();
        void OnTransformApplied(entt::entity entity);

        [[nodiscard]] bool IsPlaceMode() const;
        [[nodiscard]] bool IsEditMode() const;
        [[nodiscard]] EditorEditState* CurrentEditState();
        [[nodiscard]] const EditorEditState* CurrentEditState() const;

        EditorModeStateMachine(EditorScene& scene, EditorTransformEditor& transformEditor);
    };
} // namespace sage::editor
