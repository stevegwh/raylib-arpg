//
// Created by Steve Wheeler on 07/06/2024.
//

#include "StateMachineSystem.hpp"

namespace sage
{

StateMachineSystem::StateMachineSystem(entt::registry *_registry)
    : BaseSystem<StateMachineComponent>(_registry)
{

}
} // sage