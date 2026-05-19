#pragma once

#include "AbilityStates.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class AbilityStateMachine final : public sage::StateMachineBase<AbilityStateMachine, AbilityState>
    {
        using Base = sage::StateMachineBase<AbilityStateMachine, AbilityState>;
        friend Base;
        friend struct AbilityIdleState;
        friend struct AbilityCursorSelectState;
        friend struct AbilityAwaitingExecutionState;

        Systems* sys;

        template <typename State>
        void onEnter(State& state, const entt::entity entity)
        {
            state.OnEnter(*this, entity);
        }

        template <typename State>
        void onExit(State& state, const entt::entity entity)
        {
            state.OnExit(*this, entity);
        }

        void enableCursor(entt::entity entity);
        void disableCursor(entt::entity entity);

        void startCast(entt::entity entity);
        void cancelCast(entt::entity entity);
        void spawnAbility(entt::entity entity);
        void executeAbility(entt::entity entity);
        [[nodiscard]] bool checkRange(entt::entity entity) const;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity) const
        {
        }

      public:
        void Update();
        void Draw3D();

        ~AbilityStateMachine() = default;
        AbilityStateMachine(const AbilityStateMachine&) = delete;
        AbilityStateMachine& operator=(const AbilityStateMachine&) = delete;
        AbilityStateMachine(entt::registry* _registry, Systems* _sys);
    };
} // namespace lq
