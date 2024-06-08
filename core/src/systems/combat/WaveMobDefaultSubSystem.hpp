//
// Created by steve on 07/06/2024.
//

#pragma once

#include <entt/entt.hpp>

#include "../StateMachineSystem.hpp"

namespace sage
{

class WaveMobDefaultSystem
{
    entt::registry* registry;
    StateMachineSystem* stateMachineSystem;
public:
    WaveMobDefaultSystem(entt::registry* _registry, StateMachineSystem* _stateMachineSystem);
};

} // sage
