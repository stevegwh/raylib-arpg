//
// Created by Steve Wheeler on 23/02/2024.
//

#include "WorldSystem.hpp"
#include <memory>
#include <iostream>


namespace sage
{    
    void WorldSystem::AddComponent(sage::WorldObject& component)
    {
        components.emplace(component.entityId, std::make_unique<WorldObject>(component));
        // Not sure if I am able to modify the component object after I call make_unique
        if (component.entityId != root)
        {
            if (component.parent == 0)
            {
                component.parent = root;
            }
            components.at(root)->children.push_back(component.entityId);
        }
        
    }
}