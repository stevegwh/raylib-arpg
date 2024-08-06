//
// Created by Steve Wheeler on 01/08/2024.
//

#pragma once

#include "StateMachineComponent.hpp"
#include <tuple>

namespace sage
{

    class StateGameDefault : public StateMachineComponent
    {
      public:
        ~StateGameDefault() override = default;
    };

    class StateGameWave : public StateMachineComponent
    {
      public:
        ~StateGameWave() override = default;
    };

    using GameStates = std::tuple<StateGameDefault, StateGameWave>;
} // namespace sage