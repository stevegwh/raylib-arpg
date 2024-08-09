#pragma once

#include "Ability.hpp"

#include "particle/RainOfFireVFX.hpp"
#include "TextureTerrainOverlay.hpp"

#include <memory>

namespace sage
{
    class NavigationGridSystem;
    class Cursor;
    class Camera;

    enum class AbilityState;

    template <typename T>
    struct State
    {
        T* ability;
        virtual ~State() = default;
        virtual void Update(entt::entity self) = 0;
        virtual void Draw3D(entt::entity self) = 0;
        virtual void OnEnter(entt::entity self) = 0;
        virtual void OnExit(entt::entity self) = 0;
        State(T* _ability) : ability(_ability)
        {
        }
    };

    template <typename T>
    class IdleState : public State<T>
    {
      public:
        void Update(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void OnEnter(entt::entity self) override;
        void OnExit(entt::entity self) override;
        IdleState(T* _ability) : State<T>(_ability)
        {
        }
    };

    template <typename T>
    class CursorSelectState : public State<T>
    {
        bool cursorActive = false;
        void EnableCursor();
        void DisableCursor();

      public:
        void Update(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void OnEnter(entt::entity self) override;
        void OnExit(entt::entity self) override;
        CursorSelectState(T* _ability) : State<T>(_ability)
        {
        }
    };

    template <typename T>
    class AwaitingExecutionState : public State<T>
    {
      public:
        void Update(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void OnEnter(entt::entity self) override;
        void OnExit(entt::entity self) override;
        AwaitingExecutionState(T* _ability) : State<T>(_ability)
        {
        }
    };

    class RainOfFireAbility : public Ability
    {
        std::unordered_map<AbilityState, std::unique_ptr<State<RainOfFireAbility>>>
            states;
        State<RainOfFireAbility>* state;
        Timer animationDelayTimer{};
        std::unique_ptr<RainOfFireVFX> vfx;
        Cursor* cursor;
        std::unique_ptr<TextureTerrainOverlay> spellCursor;
        float whirlwindRadius = 50.0f;
        void Confirm(entt::entity self);

      public:
        void Init(entt::entity self) override;
        void Execute(entt::entity self) override;
        void Draw3D(entt::entity self) override;
        void Update(entt::entity self) override;
        ~RainOfFireAbility() override = default;
        RainOfFireAbility(
            entt::registry* _registry,
            Camera* _camera,
            Cursor* _cursor,
            NavigationGridSystem* _navigationGridSystem);

        friend class IdleState<RainOfFireAbility>;
        friend class CursorSelectState<RainOfFireAbility>;
        friend class AwaitingExecutionState<RainOfFireAbility>;
    };
} // namespace sage