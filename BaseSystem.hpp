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

namespace sage
{
    template <typename ComponentName>
    class BaseSystem
    {
    private:
        const std::string componentName;
    protected:
        std::unordered_map<EntityID, std::unique_ptr<ComponentName>> components;
        std::vector<std::shared_ptr<EventCallback>> eventCallbacks;
        
        void m_addComponent(std::unique_ptr<ComponentName> component)
        {
            // Subscribe to parent entity's "OnDelete" event
            const std::function<void()> f1 = [p = this, id = component->entityId] { p->RemoveComponent(id); };
            auto e1 = std::make_shared<EventCallback>(f1);
            eventCallbacks.push_back(e1);
            Registry::GetInstance().GetEntity(component->entityId)->OnDelete->Subscribe(e1);

            components.emplace(component->entityId, std::move(component));
        }
        
    public:

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
