//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "systems/BaseSystem.hpp"
#include "components/states/StateMachineComponent.hpp"
#include "systems/StateMachineSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "WaveMobDefaultSubSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{

class DefaultStateSystem : public BaseSystem<StateMachineComponent>
{
    StateMachineSystem* stateMachineSystem;
    std::unique_ptr<WaveMobDefaultSubSystem> waveMobDefaultSubSystem;
public:
    void Update();
    DefaultStateSystem(entt::registry* _registry, 
                       StateMachineSystem* _stateMachineSystem,
                       TransformSystem* _transformSystem);
};

} // sage
