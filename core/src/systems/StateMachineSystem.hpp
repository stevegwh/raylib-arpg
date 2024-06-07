//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "../components/StateMachineComponent.hpp"

#include <entt/entt.hpp>

namespace sage
{

class StateMachineSystem : BaseSystem<StateMachineComponent>
{
public:
    explicit StateMachineSystem(entt::registry* _registry);
};

} // sage
