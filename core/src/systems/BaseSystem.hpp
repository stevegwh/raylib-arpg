//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <memory>
#include <vector>
#include <string>

#include "entt/entt.hpp"

namespace sage
{
    template <typename ComponentName>
    class BaseSystem
    {
    private:
        const std::string componentName;
        
    protected:
        //std::unique_ptr<EventManager> eventManager;
        entt::registry* registry;
        
    public:

        BaseSystem(entt::registry* _registry) : registry(_registry)
        //, eventManager(std::make_unique<EventManager>()) 
        {}

        [[nodiscard]] const char* getComponentName() const
        {
            return typeid(ComponentName).name();
        }
    };
}
