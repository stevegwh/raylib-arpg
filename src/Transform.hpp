//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <memory>
#include <queue>

#include "raylib.h"

#include "Component.hpp"
#include "TransformSystem.hpp"

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

        friend class TransformSystem;
        
        [[nodiscard]] std::unordered_map<std::string, std::string> SerializeImpl() const
        {
            return {
                {"EntityId", TextFormat("%i", entityId)},
                {"Position", TextFormat("%02.02f, %02.02f, %02.02f", position.x, position.y, position.z)}
            };
        }
        
        std::unique_ptr<Event> OnPositionUpdate;
        std::unique_ptr<Event> OnStartMovement;
        std::unique_ptr<Event> OnFinishMovement;
        
        explicit Transform(EntityID _entityId) : 
        OnPositionUpdate(std::make_unique<Event>()), 
        OnStartMovement(std::make_unique<Event>()),
        OnFinishMovement(std::make_unique<Event>()),
        Component(_entityId) {}
        
        void positionSet(const Vector3& pos)
        {
            position = pos;
            OnPositionUpdate->InvokeAllCallbacks();
        }

    };
}

