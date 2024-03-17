//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <memory>
#include <queue>

#include "raylib.h"

#include "Component.hpp"

namespace sage
{
    struct Transform : public Component
    {
        Vector3 position{};
        std::queue<Vector3> targets{};
        Vector3 direction{};
        float scale{};
        Vector3 rotation{};

        explicit Transform(EntityID _entityId) : Component(_entityId) {}
    };
}

