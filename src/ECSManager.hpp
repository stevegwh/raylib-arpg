//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

// Systems
#include "RenderSystem.hpp"
#include "CollisionSystem.hpp"
#include "TransformSystem.hpp"
#include "WorldSystem.hpp"
#include "NavigationGridSystem.hpp"
#include "ActorMovementSystem.hpp"
#include "AnimationSystem.hpp"

#include <entt/entt.hpp>

#include <memory>

namespace sage
{
class ECSManager
{
    entt::registry* registry;
public:
    ECSManager(entt::registry* _registry, UserInput* userInput);
    
    std::unique_ptr<sage::CollisionSystem> collisionSystem;
    std::unique_ptr<sage::RenderSystem> renderSystem;
    std::unique_ptr<sage::TransformSystem> transformSystem;
    std::unique_ptr<sage::WorldSystem> worldSystem;
    std::unique_ptr<sage::NavigationGridSystem> navigationGridSystem;
    std::unique_ptr<sage::ActorMovementSystem> actorMovementSystem;
    std::unique_ptr<sage::AnimationSystem> animationSystem;

    void DeserializeMap();
    void SerializeMap() const;

};

}

