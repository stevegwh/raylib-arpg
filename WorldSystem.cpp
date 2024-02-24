//
// Created by Steve Wheeler on 23/02/2024.
//

#include "WorldSystem.hpp"
#include <memory>


namespace sage
{    
    void WorldSystem::AddComponent(std::unique_ptr<WorldObject> component)
    {
        // Subscribe to parent entity's "OnDelete" event
        const std::function<void()> f1 = [p = this, id = component->entityId] { p->RemoveComponent(id); };
        Registry::GetInstance().GetEntity(component->entityId)->OnDelete->Subscribe(std::make_shared<EventCallback>(f1));
        
        if (component->entityId != root)
        {
            if (component->parent == 0)
            {
                component->parent = root;
            }
            components.at(root)->children.push_back(component->entityId);
        }

        components.emplace(component->entityId, std::move(component));
        
    }
}