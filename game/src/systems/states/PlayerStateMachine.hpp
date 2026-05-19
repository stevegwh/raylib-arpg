// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "PlayerStates.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class PlayerStateMachine final : public sage::StateMachineBase<PlayerStateMachine, PlayerState>
    {
        using Base = StateMachineBase<PlayerStateMachine, PlayerState>;
        friend Base;
        friend struct PlayerDefaultState;
        friend struct PlayerMovingToLocationState;
        friend struct PlayerMovingToAttackEnemyState;
        friend struct PlayerMovingToTalkState;
        friend struct PlayerMovingToLootState;
        friend struct PlayerInDialogState;
        friend struct PlayerCombatState;

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

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);
        void bindCursorInput(entt::entity entity);

        void onFloorClick(entt::entity entity, entt::entity);
        void onChestClick(entt::entity entity, entt::entity target);
        void onNPCLeftClick(entt::entity entity, entt::entity target);
        void onEnemyLeftClick(entt::entity entity, entt::entity target);

      public:
        void Update();
        void Draw3D();

        ~PlayerStateMachine() = default;
        PlayerStateMachine(entt::registry* _registry, Systems* sys);
    };
} // namespace lq
