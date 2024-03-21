//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "Component.hpp"

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

#include "Registry.hpp"

namespace sage
{
    template <typename ComponentName>
    class BaseSystem
    {
    protected:
        std::unordered_map<EntityID, std::unique_ptr<ComponentName>> components;
        
        void m_addComponent(std::unique_ptr<ComponentName> component)
        {
            // Subscribe to parent entity's "OnDelete" event
            const std::function<void()> f1 = [p = this, id = component->entityId] { p->RemoveComponent(id); };
            Registry::GetInstance().GetEntity(component->entityId)->OnDelete->Subscribe(std::make_shared<EventCallback>(f1));

            components.emplace(component->entityId, std::move(component));
        }
        
    public:
        
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

        std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>> SerializeComponents()
        {
            std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>> toReturn;
            toReturn.first = "Transform";
            for (const auto& c: components) 
            {
                toReturn.second.push_back(c.second->Serialize());
            }

            return toReturn;
        }
        
        void DeserializeComponents()
        {
            
        }
    };
}
