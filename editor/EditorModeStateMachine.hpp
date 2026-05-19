#pragma once

#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"
#include "raylib.h"
#include "raymath.h"

#include <cstddef>
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

        [[nodiscard]] bool IsPlaceMode() const;
        [[nodiscard]] bool IsEditMode() const;
        [[nodiscard]] EditorEditState* CurrentEditState();
        [[nodiscard]] const EditorEditState* CurrentEditState() const;

        EditorModeStateMachine(entt::registry* registry, EditorScene& scene, EditorTransformEditor& transformEditor);
    };
} // namespace sage::editor
