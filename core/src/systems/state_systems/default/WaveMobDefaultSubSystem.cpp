//
// Created by Steve Wheeler on 08/06/2024.
//

#include "WaveMobDefaultSubSystem.hpp"
#include "components/states/StateEnemyDefault.hpp"
#include "components/Transform.hpp"
#include "components/Animation.hpp"

#include <iostream>

namespace sage
{

void WaveMobDefaultSubSystem::Update()
{
    auto view = registry->view<StateEnemyDefault, MoveableActor>();
    for (auto& entity: view) 
    {
        auto& a = registry->get<MoveableActor>(entity);
        if (a.globalPath.empty())
        {
            auto& animation = registry->get<Animation>(entity);
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
            transformSystem->PathfindToLocation(entity, {{0,0,-50}}); // Temporary. Just move to location
        }
        // Set animation to walking
        // Move towards nexus
        // If within range of nexus, set animation to attack

    }
}

WaveMobDefaultSubSystem::WaveMobDefaultSubSystem(entt::registry* _registry,
                                                 StateMachineSystem* _stateMachineSystem,
                                                 ActorMovementSystem* _transformSystem) :
    registry(_registry), 
    stateMachineSystem(_stateMachineSystem),
    transformSystem(_transformSystem)
{
    
}
} // sage