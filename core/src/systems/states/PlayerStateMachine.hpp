// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"
#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    class PlayerStateController final
        : public StateMachineController<PlayerStateController, PlayerState, PlayerStateEnum>
    {
        class DefaultState;
        class MovingToAttackEnemyState;
        class MovingToTalkToNPCState;
        class InDialogState;
        class CombatState;
        class FollowingLeaderState;

      public:
        void Update();
        void Draw3D();

        ~PlayerStateController() override = default;
        PlayerStateController(entt::registry* _registry, GameData* gameData);

        friend class StateMachineController; // Required for CRTP
    };
} // namespace sage