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
   
    class StatePlayerApproachingTarget : public StateMachineComponent
    {
    public:
        ~StatePlayerApproachingTarget() override = default;
    };

    class StatePlayerEngagedInCombat : public StateMachineComponent
    {
    public:
        ~StatePlayerEngagedInCombat() override = default;
    };

    using PlayerStates = std::tuple<
        StatePlayerDefault,
        StatePlayerApproachingTarget,
        StatePlayerEngagedInCombat,
        StatePlayerCombatCooldown,
        StatePlayerRetreating
    >;
} // sage