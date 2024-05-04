//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ECSManager.hpp"
#include "Serializer.hpp"

namespace sage
{
ECSManager::ECSManager(entt::registry* _registry, UserInput* userInput) :
    registry(_registry),
    renderSystem(std::make_unique<RenderSystem>(_registry)),
    collisionSystem(std::make_unique<sage::CollisionSystem>(_registry)),
    transformSystem(std::make_unique<sage::TransformSystem>(_registry)),
    navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry)),
    animationSystem(std::make_unique<AnimationSystem>(_registry))
{
//    entt::entity rootNodeId = registry->create();
//    auto rootNodeObject = std::make_unique<WorldObject>();
//    worldSystem = std::make_unique<sage::WorldSystem>(_registry, rootNodeId);
//    registry->emplace<WorldObject>(rootNodeId, new WorldObject());
    actorMovementSystem = std::make_unique<sage::ActorMovementSystem>(_registry, userInput);
}

void ECSManager::Load()
{
    serializer::Load(registry);
}

void ECSManager::Save() const
{
    serializer::Save(*registry);
}
}
