// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "entt/entt.hpp"
#include "systems/states/StateMachine.hpp"

namespace sage
{
    class Systems;

    class PlayerStateMachine final : public StateMachine<PlayerState, PlayerStateEnum>
    {
        class DefaultState;
        class MovingToLocationState;
        class MovingToAttackEnemyState;
        class MovingToTalkToNPCState;
        class MovingToLootState;
        class InDialogState;
        class CombatState;
        class DestinationUnreachableState;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

        void onFloorClick(entt::entity self, entt::entity);
        void onChestClick(entt::entity self, entt::entity);
        void onNPCLeftClick(entt::entity self, entt::entity target);
        void onEnemyLeftClick(entt::entity self, entt::entity target);

      public:
        void Update();
        void Draw3D();

        ~PlayerStateMachine() override = default;
        PlayerStateMachine(entt::registry* _registry, Systems* sys);

        friend class StateMachine; // Required for CRTP
    };
} // namespace sage