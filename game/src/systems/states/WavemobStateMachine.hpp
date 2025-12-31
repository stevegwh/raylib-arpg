// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachine.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class WavemobStateMachine : public sage::StateMachine<WavemobState, WavemobStateEnum>
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
} // namespace lq