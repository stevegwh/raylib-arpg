//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <memory>
#include <queue>

#include "raylib.h"

#include "Component.hpp"

#include "Event.hpp"

namespace sage
{
    struct Transform : public Component<Transform>
    {
        Vector3 position{};
        std::queue<Vector3> targets{};
        Vector3 direction{};
        float scale = 1.0f;
        Vector3 rotation{};
        
        [[nodiscard]] std::unordered_map<std::string, std::string> SerializeImpl() const
        {
            return {
                {"EntityId", TextFormat("%i", entityId)},
                {"Position", TextFormat("%02.02f, %02.02f, %02.02f", position.x, position.y, position.z)}
            };
        }
        std::unique_ptr<Event> OnPositionUpdate;

        explicit Transform(EntityID _entityId) : OnPositionUpdate(std::make_unique<Event>()), Component(_entityId) {}
    };
}

