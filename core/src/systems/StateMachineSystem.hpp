//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "components/states/StateMachineComponent.hpp"

#include <entt/entt.hpp>

namespace sage
{

class StateMachineSystem : BaseSystem<StateMachineComponent>
{
public:
    template<typename NewStateComponent>
    void ChangeState(entt::entity entity)
    {
        if (registry->any_of<NewStateComponent>(entity)) return;
        auto view = registry->view<StateMachineComponent>();
        if (view.contains(entity))
        {
            view.each([&](auto e, auto& component)
                      {
                          if (e == entity)
                          {
                              registry->remove<std::decay_t<decltype(component)>>(entity);
                              component.Disable(entity);
                          }
                      });
        }
        auto& newComponent = registry->emplace<NewStateComponent>(entity);
        newComponent.Enable(entity);
    }
    explicit StateMachineSystem(entt::registry* _registry);
};

} // sage
