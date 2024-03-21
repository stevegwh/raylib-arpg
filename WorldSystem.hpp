//
// Created by Steve Wheeler on 23/02/2024.
//

#pragma once
#include "WorldObject.hpp"
#include "BaseSystem.hpp"

namespace sage
{
    class WorldSystem : public BaseSystem<WorldObject>
    {
        EntityID root;
        
    public:

        void AddComponent(std::unique_ptr<WorldObject> component);
        
        explicit WorldSystem(EntityID rootNodeId) : BaseSystem<WorldObject>("WorldObject"), root(rootNodeId) {}
    };
}


