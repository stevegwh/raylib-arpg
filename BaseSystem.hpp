//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once
#include "Component.hpp"
#include "map"
#include "memory"

namespace sage
{
    template <typename ComponentName>
    class BaseSystem
    {
    protected:
        std::map<EntityID, std::unique_ptr<ComponentName>> components;
    public:
        
        [[nodiscard]] bool EntityExists(EntityID entityId) const
        {
            return components.find(entityId) != components.end();
        };

        const ComponentName& GetComponent(EntityID entityId) const
        {
            return *components.at(entityId);
        }

        void AddComponent(ComponentName &component)
        {
            components.emplace(component.entityId, std::make_unique<ComponentName>(component));
        }
    };
}
