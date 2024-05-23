//
// Created by Steve Wheeler on 27/03/2024.
//

#include "GameData.hpp"
#include "../utils/Serializer.hpp"

namespace sage
{
GameData::GameData(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings) :
    registry(_registry),
    settings(_settings),
    renderSystem(std::make_unique<RenderSystem>(_registry)),
    collisionSystem(std::make_unique<sage::CollisionSystem>(_registry)),
    transformSystem(std::make_unique<sage::TransformSystem>(_registry)),
    navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry)),
    animationSystem(std::make_unique<AnimationSystem>(_registry))
{
    userInput = std::make_unique<UserInput>(_keyMapping, settings);
    camera = std::make_unique<sage::Camera>(userInput.get());
    cursor = std::make_unique<Cursor>(registry,
                                      collisionSystem.get(),
                                      navigationGridSystem.get(),
                                      camera.get(),
                                      userInput.get());
    
    actorMovementSystem = std::make_unique<sage::ActorMovementSystem>(_registry,
                                                                      cursor.get(),
                                                                      userInput.get(),
                                                                      navigationGridSystem.get(), 
                                                                      transformSystem.get());
    dialogueSystem = std::make_unique<sage::DialogueSystem>(_registry, 
                                                            cursor.get(), 
                                                            camera.get(),
                                                            settings,
                                                            actorMovementSystem.get());
    combatSystem = std::make_unique<sage::HealthBarSystem>(_registry, camera.get());
    {
        entt::sink sink{actorMovementSystem->onControlledActorChange};
        sink.connect<&Cursor::OnControlledActorChange>(*cursor);
    }
}

void GameData::Load()
{
    serializer::Load(registry);
}

void GameData::Save() const
{
    serializer::Save(*registry);
}
}
