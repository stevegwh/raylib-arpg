//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once
#include "StateMachineComponent.hpp"

namespace sage
{

class StatePlayerCombat : public StateMachineComponent
{
public:
    StatePlayerCombat() = default;
    ~StatePlayerCombat() override = default;
    bool CheckValidity() override
    {
        // Implement the logic to check if the state is still valid
        // Return true if the state is valid, false otherwise
        return true;
    }

    void Enable() override
    {
        // Implement the logic to enable the state
    }

    void Disable() override
    {
        // Implement the logic to disable the state
    }
};

} // sage
