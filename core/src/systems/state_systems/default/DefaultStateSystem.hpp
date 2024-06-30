//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "systems/BaseSystem.hpp"
#include "components/states/StateMachineComponent.hpp"
#include "systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "WaveMobDefaultSubSystem.hpp"
#include "PlayerDefaultSubSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{

class DefaultStateSystem : public BaseSystem<StateMachineComponent>
{
    StateMachineSystem* stateMachineSystem;
    std::unique_ptr<WaveMobDefaultSubSystem> waveMobDefaultSubSystem;
    std::unique_ptr<PlayerDefaultSubSystem> playerDefaultSubSystem;
public:
    void Update();
    DefaultStateSystem(entt::registry* _registry,
                       StateMachineSystem* _stateMachineSystem,
                       ActorMovementSystem* _actorMovementSystem);
};

} // sage
