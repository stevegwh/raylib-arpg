//
// Created by Steve Wheeler on 01/08/2024.
//
#pragma once
#include "StateMachineComponent.hpp"
#include <tuple>

namespace sage
{
    class StatePlayerDefault : public StateMachineComponent
    {
      public:
        ~StatePlayerDefault() override = default;
    };

    class StatePlayerMovingToAttackEnemy : public StateMachineComponent
    {
      public:
        ~StatePlayerMovingToAttackEnemy() override = default;
    };

    class StatePlayerMovingToTalkToNPC : public StateMachineComponent
    {
      public:
        ~StatePlayerMovingToTalkToNPC() override = default;
    };

    class StatePlayerCombat : public StateMachineComponent
    {
      public:
        ~StatePlayerCombat() override = default;
    };

    using PlayerStates = std::tuple<
        StatePlayerDefault,
        StatePlayerMovingToAttackEnemy,
        StatePlayerMovingToTalkToNPC,
        StatePlayerCombat>;
} // namespace sage