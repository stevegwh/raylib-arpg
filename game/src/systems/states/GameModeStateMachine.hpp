//
// Created by Steve Wheeler on 31/07/2024.
//

#pragma once

#include "GameModeStates.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

#include <utility>

namespace lq
{
    class GameModeStateMachine final : public sage::StateMachineBase<GameModeStateMachine, GameState>
    {
        using Base = sage::StateMachineBase<GameModeStateMachine, GameState>;
        friend Base;
        friend struct GameDefaultState;
        friend struct GameWaveState;
        friend struct GameCombatState;

        entt::entity gameEntity;

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

      public:
        // No-entity convenience overload — the game state machine has a single, internally
        // owned entity. Hides Base::ChangeState(entity, NewState) inside this class scope,
        // which is intentional: callers should never need to know about gameEntity.
        template <typename NewState>
        void ChangeState(NewState newState = {})
        {
            Base::ChangeState(gameEntity, std::move(newState));
        }

        void StartCombat();
        void Update();
        void Draw3D();

        ~GameModeStateMachine() = default;
        explicit GameModeStateMachine(entt::registry* _registry);
    };

} // namespace lq
