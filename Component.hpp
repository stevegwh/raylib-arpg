//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

typedef int EntityID;

namespace sage
{
    struct Component
    {
        const EntityID entityId;
        explicit Component(EntityID _entityId) : entityId(_entityId) {}
    };
}
