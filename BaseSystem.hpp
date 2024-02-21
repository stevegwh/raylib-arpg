//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once
#include "Component.hpp"
#include "map"
#include "memory"

namespace sage
{
    template <typename Derived>
    class BaseSystem
    {
    protected:
        std::map<EntityID, std::unique_ptr<Derived>> components;
    public:
        
        bool EntityExists(EntityID entityId) 
        {
            return components.find(entityId) != components.end();
        };

        const Derived& GetComponent(EntityID entityId)
        {
            return *components.at(entityId);
        }

        void AddComponent(Derived &component)
        {
            components.emplace(component.entityId, std::make_unique<Derived>(component));
        }
    
    
    };
}
