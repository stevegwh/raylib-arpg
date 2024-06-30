// Created by Steve Wheeler on 30/06/2024.

#pragma once

#include "entt/entt.hpp"

#include "systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"

namespace sage
{

class PlayerDefaultSubSystem
{
    entt::registry* registry;
    StateMachineSystem* stateMachineSystem;
    ActorMovementSystem* actorMovementSystem;
public:
    void Update();
    void OnComponentEnabled(entt::entity entity);
    void OnComponentDisabled(entt::entity entity);
    PlayerDefaultSubSystem(entt::registry* _registry,
                            StateMachineSystem* _stateMachineSystem,
                            ActorMovementSystem* _actorMovementSystem);
};

} // sage