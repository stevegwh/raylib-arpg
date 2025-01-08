// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class Systems;

    class WavemobStateController : public StateMachineController<WavemobState, WavemobStateEnum>
    {
        class DefaultState;
        class TargetOutOfRangeState;
        class CombatState;
        class DyingState;

      public:
        void Update();
        void Draw3D();
        ~WavemobStateController() override = default;
        WavemobStateController(entt::registry* _registry, Systems* _sys);

        friend class StateMachineController; // Required for CRTP
    };
} // namespace sage