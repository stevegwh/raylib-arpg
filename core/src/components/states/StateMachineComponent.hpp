//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once

namespace sage
{

struct StateMachineComponent
{
    // All the state component should do is check if it's still valid.
    // If it isn't, then flag the state as invalid.
    // The FSM will then remove it from its corresponding system and move active state to NextState/Default.
    // The issue with a tag system is there could be multiple competing states at any one time (not a FSM)
    StateMachineComponent* PreviousState{};
    StateMachineComponent* NextState{};
    virtual bool CheckValidity() = 0;
    virtual void Enable() = 0;
    virtual void Disable() = 0;
    virtual ~StateMachineComponent() = default;
};
}
