//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "Component.hpp"

#include <unordered_map>
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <typeinfo>

#include "Registry.hpp"
#include "EventManager.hpp"

namespace sage
{
    template <typename ComponentName>
    class BaseSystem
    {
    private:
        const std::string componentName;
        
    protected:
        std::unique_ptr<EventManager> eventManager;
        std::unordered_map<EntityID, std::unique_ptr<ComponentName>> components;
        void m_addComponent(std::unique_ptr<ComponentName> component)
        {
            eventManager->Subscribe([p = this, id = component->entityId]
                                   { p->RemoveComponent(id); },
                                    *Registry::GetInstance().GetEntity(component->entityId)->OnDelete);
            components.emplace(component->entityId, std::move(component));
        }
        
    public:

        BaseSystem() : eventManager(std::make_unique<EventManager>()) {}

        const char* getComponentName() const
        {
            return typeid(ComponentName).name();
        }
        
        [[nodiscard]] bool FindEntity(EntityID entityId) const
        {

            return components.find(entityId) != components.end();
        };

        const ComponentName* GetComponent(EntityID entityId) const
        {
            return components.at(entityId).get();
        }
        
        const std::unordered_map<EntityID, std::unique_ptr<ComponentName>>& GetComponents() const
        {
            return components;
        }

        // Hideable
        void AddComponent(std::unique_ptr<ComponentName> component)
        {
            m_addComponent(std::move(component));
        }
        
        void RemoveComponent(EntityID entityId)
        {
            components.erase(entityId);
        }

//        {
//            EntityID  -> { ComponentName -> { Field -> Value} }
//        }
        void SerializeComponents(std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>& serializeData) const
        {
            
            for (const auto& c : components)
            {
                if (!Registry::GetInstance().GetEntity(c.first)->serializable) continue;
                serializeData[std::to_string(c.first)][getComponentName()] = c.second->Serialize();
            }
        }
    };
}
