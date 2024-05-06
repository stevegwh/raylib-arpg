//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ECSManager.hpp"
#include "../utils/Serializer.hpp"

namespace sage
{
ECSManager::ECSManager(entt::registry* _registry, KeyMapping _keyMapping) :
    registry(_registry),
    renderSystem(std::make_unique<RenderSystem>(_registry)),
    collisionSystem(std::make_unique<sage::CollisionSystem>(_registry)),
    transformSystem(std::make_unique<sage::TransformSystem>(_registry)),
    navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry)),
    animationSystem(std::make_unique<AnimationSystem>(_registry))
{
    userInput = std::make_unique<UserInput>(_keyMapping);
    camera = std::make_unique<sage::Camera>(userInput.get());
    cursor = std::make_unique<Cursor>(registry, collisionSystem.get(), camera.get());
    
    actorMovementSystem = std::make_unique<sage::ActorMovementSystem>(_registry,
                                                                      cursor.get(),
                                                                      userInput.get(), 
                                                                      navigationGridSystem.get(), 
                                                                      transformSystem.get());
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
