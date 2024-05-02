//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "Component.hpp"

#include <memory>
#include <vector>
#include <string>

#include <entt/entt.hpp>

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
        
        void SerializeComponents(std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>& serializeData) const
        {
            
//            for (const auto& c : components)
//            {
//                if (!Registry::GetInstance().GetEntity(c.first)->serializable) continue;
//                serializeData[std::to_string(c.first)][getComponentName()] = c.second->Serialize();
//            }
        }
    };
}
