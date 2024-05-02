//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

#include <memory>
#include <queue>

#include "Component.hpp"
#include "TransformSystem.hpp"
#include "Event.hpp"

namespace sage
{
    struct Transform
    {
        Vector3 position{};
        std::queue<Vector3> targets{};
        Vector3 direction{};
        float scale = 1.0f;
        Vector3 rotation{};

        friend class TransformSystem;
        
        [[nodiscard]] std::unordered_map<std::string, std::string> SerializeImpl() const
        {
//            return {
//                {"EntityId", TextFormat("%i", entityId)},
//                {"Position", TextFormat("%02.02f, %02.02f, %02.02f", position.x, position.y, position.z)}
//            };
            return {};
        }

        entt::delegate<void(entt::entity)> dOnPositionUpdate{};
        entt::delegate<void(entt::entity)> dOnStartMovement{};
        entt::delegate<void(entt::entity)> dOnFinishMovement{};

    };
}

