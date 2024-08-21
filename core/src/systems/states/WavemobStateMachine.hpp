// Created by Steve Wheeler on 30/06/2024.
#pragma once
#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    class GameData;
    struct AttackData;

    class WavemobStateController
        : public StateMachineController<WavemobStateController, WavemobState, WavemobStateEnum>
    {

        class DefaultState;
        class TargetOutOfRangeState;
        class CombatState;
        class DyingState;

        DefaultState* defaultState;
        TargetOutOfRangeState* targetOutOfRangeState;
        CombatState* combatState;
        DyingState* dyingState;

      protected:
        StateMachine* GetSystem(WavemobStateEnum state) override;

      public:
        WavemobStateController(entt::registry* _registry, GameData* gameData);
        void Update();
        void Draw3D();

        friend class StateMachineController; // Required for CRTP
    };
} // namespace sage