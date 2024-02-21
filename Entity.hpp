//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <utility>

//#include "EntityRegistry.hpp"
namespace sage
{
    typedef int EntityID;
    
    struct Entity
    {
        static EntityID entityIdCounter;
        const EntityID entityId;
        explicit Entity()
        : entityId(++entityIdCounter)
        {}
    };
}

