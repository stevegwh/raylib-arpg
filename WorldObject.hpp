//
// Created by Steve Wheeler on 23/02/2024.
//

#pragma once
#include <vector>
#include "Component.hpp"

namespace sage
{
    struct WorldObject : sage::Component
    {
        EntityID parent{};
        std::vector<EntityID> children;
        explicit WorldObject(EntityID _entityId) : Component(_entityId) {}
    };
}

