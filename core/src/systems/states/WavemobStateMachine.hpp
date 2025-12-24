// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class Systems;

    class WavemobStateMachine : public StateMachine<WavemobState, WavemobStateEnum>
    {
        class DefaultState;
        class TargetOutOfRangeState;
        class CombatState;
        class DyingState;

      public:
        void Update();
        void Draw3D();
        ~WavemobStateMachine() override = default;
        WavemobStateMachine(entt::registry* _registry, Systems* _sys);

        friend class StateMachine; // Required for CRTP
    };
} // namespace sage