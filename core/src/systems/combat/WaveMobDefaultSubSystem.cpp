//
// Created by steve on 07/06/2024.
//

#include "WaveMobDefaultSubSystem.hpp"

namespace sage
{
WaveMobDefaultSystem::WaveMobDefaultSystem(entt::registry *_registry, StateMachineSystem *_stateMachineSystem) :
    registry(_registry), stateMachineSystem(_stateMachineSystem)
{

}
} // sage