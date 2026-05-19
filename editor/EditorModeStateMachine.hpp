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
    struct EditorSelectState
    {
    };

    struct EditorPlaceState
    {
        std::size_t placeableIndex = 0;
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

        EditorScene& scene;
        entt::entity stateEntity = entt::null;

        void onEnter(EditorSelectState&, entt::entity);
        void onExit(EditorSelectState&, entt::entity);
        void update(EditorSelectState&, entt::entity);
        void draw3D(const EditorSelectState&, entt::entity) const;

        void onEnter(EditorPlaceState&, entt::entity);
        void onExit(EditorPlaceState&, entt::entity);
        void update(EditorPlaceState&, entt::entity);
        void draw3D(const EditorPlaceState&, entt::entity) const;

        void onEnter(EditorEditState&, entt::entity);
        void onExit(EditorEditState&, entt::entity);
        void update(EditorEditState&, entt::entity);
        void draw3D(const EditorEditState&, entt::entity) const;

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
        [[nodiscard]] const EditorEditState* CurrentEditState() const;

        EditorModeStateMachine(entt::registry* registry, EditorScene& scene);
    };
} // namespace sage::editor
