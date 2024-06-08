//
// Created by Steve Wheeler on 08/06/2024.
//

#include "DefaultStateSystem.hpp"

namespace sage
{
void DefaultStateSystem::Update()
{
    waveMobDefaultSubSystem->Update();
}

DefaultStateSystem::DefaultStateSystem(entt::registry* _registry, 
                                       StateMachineSystem* _stateMachineSystem,
                                       TransformSystem* _transformSystem):
                                       BaseSystem<StateMachineComponent>(_registry),
                                       stateMachineSystem(_stateMachineSystem),
                                       waveMobDefaultSubSystem(std::make_unique<WaveMobDefaultSubSystem>(_registry, 
                                                                                                         _stateMachineSystem,
                                                                                                         _transformSystem))
{

}
} // sage