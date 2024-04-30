//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Component.hpp"

namespace sage
{
    struct Actor : public Component<Actor>
    {
        explicit Actor(EntityID entityId) : Component(entityId) {}
    };
}
