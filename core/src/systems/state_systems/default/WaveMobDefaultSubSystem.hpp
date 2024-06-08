//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "systems/StateMachineSystem.hpp"
#include "systems/TransformSystem.hpp"

namespace sage
{

class WaveMobDefaultSubSystem
{
    entt::registry* registry;
    StateMachineSystem* stateMachineSystem;
    TransformSystem* transformSystem;
public:
    void Update();
    WaveMobDefaultSubSystem(entt::registry* _registry, 
                            StateMachineSystem* _stateMachineSystem,
                            TransformSystem* _transformSystem);
};

} // sage
