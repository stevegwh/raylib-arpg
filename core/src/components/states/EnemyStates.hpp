//
// Created by Steve Wheeler on 01/08/2024.
//

#pragma once

#include "StateMachineComponent.hpp"

#include <tuple>

namespace sage
{
    class StateEnemyDefault : public StateMachineComponent
    {
      public:
        ~StateEnemyDefault() override = default;
    };

    class StateEnemyCombat : public StateMachineComponent
    {
      public:
        ~StateEnemyCombat() override = default;
    };

    class StateEnemyTargetOutOfRange : public StateMachineComponent
    {
      public:
        ~StateEnemyTargetOutOfRange() override = default;
    };

    class StateEnemyDying : public StateMachineComponent
    {
      public:
        ~StateEnemyDying() override = default;
    };

    using EnemyStates = std::tuple<
        StateEnemyDefault,
        StateEnemyCombat,
        StateEnemyTargetOutOfRange,
        StateEnemyDying>;
} // namespace sage