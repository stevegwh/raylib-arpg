//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "string"

typedef int EntityID;

namespace sage
{
    template <typename ComponentName>
    struct Component
    {
        const EntityID entityId;
        explicit Component(EntityID _entityId) : entityId(_entityId) {}

        std::string Serialize()
        {
            return static_cast<ComponentName*>(this)->SerializeImpl();
        }
    };
}
