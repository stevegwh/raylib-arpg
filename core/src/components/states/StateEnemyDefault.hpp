//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once

#include "StateMachineComponent.hpp"

namespace sage
{

class StateEnemyDefault : public StateMachineComponent
{
public:
    // TODO: Add OnExit/OnEnter which subscribes to transform events (currently handled in the state systems)
    ~StateEnemyDefault() override = default;
};

} // sage
