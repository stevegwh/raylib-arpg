//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

// Systems
#include "systems/RenderSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/WorldSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
class ECSManager
{
    entt::registry* registry;
public:
    ECSManager(entt::registry* _registry, sage::Camera* sCamera);
    
    std::unique_ptr<UserInput> userInput;
    std::unique_ptr<Cursor> cursor;
    
    std::unique_ptr<sage::CollisionSystem> collisionSystem;
    std::unique_ptr<sage::RenderSystem> renderSystem;
    std::unique_ptr<sage::TransformSystem> transformSystem;
    std::unique_ptr<sage::WorldSystem> worldSystem;
    std::unique_ptr<sage::NavigationGridSystem> navigationGridSystem;
    std::unique_ptr<sage::ActorMovementSystem> actorMovementSystem;
    std::unique_ptr<sage::AnimationSystem> animationSystem;

    void Load();
    void Save() const;

};

}
