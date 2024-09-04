// Created by Steve Wheeler on 30/06/2024.
#pragma once
#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    class GameData;

    class WavemobStateController
        : public StateMachineController<WavemobStateController, WavemobState, WavemobStateEnum>
    {
        class DefaultState;
        class TargetOutOfRangeState;
        class CombatState;
        class DyingState;

      public:
        void Update();
        void Draw3D();
        ~WavemobStateController() override;
        WavemobStateController(entt::registry* _registry, GameData* _gameData);

        friend class StateMachineController; // Required for CRTP
    };
} // namespace sage