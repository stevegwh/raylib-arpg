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
    auto view = registry->view<StateEnemyDefault, Transform>();
    for (auto& entity: view) 
    {
        auto& t = registry->get<Transform>(entity);
        auto& animation = registry->get<Animation>(entity);
        animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
        transformSystem->PathfindToLocation(entity, {{0,0,0}}); // Temporary. Just move to location
        // Set animation to walking
        // Move towards nexus
        // If within range of nexus, set animation to attack

    }
}

WaveMobDefaultSubSystem::WaveMobDefaultSubSystem(entt::registry* _registry, 
                                                 StateMachineSystem* _stateMachineSystem,
                                                 TransformSystem* _transformSystem) :
    registry(_registry), 
    stateMachineSystem(_stateMachineSystem),
    transformSystem(_transformSystem)
{
    
}
} // sage