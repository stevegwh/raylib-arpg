//
// Created by Steve Wheeler on 27/03/2024.
//

#include "GameData.hpp"
#include <Serializer.hpp>

#include "Camera.hpp"
#include "Cursor.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"

// Systems
#include "abilities/Ability.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/dialogue/DialogueSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"

namespace sage
{
    GameData::GameData(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings)
        : registry(_registry),
          userInput(std::make_unique<UserInput>(_keyMapping, settings)),
          cursor(std::make_unique<Cursor>(registry, this)),
          camera(std::make_unique<Camera>(userInput.get())),
          settings(_settings),
          renderSystem(std::make_unique<RenderSystem>(_registry)),
          collisionSystem(std::make_unique<CollisionSystem>(_registry)),
          navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry, collisionSystem.get())),
          actorMovementSystem(
              std::make_unique<ActorMovementSystem>(_registry, collisionSystem.get(), navigationGridSystem.get())),
          controllableActorSystem(std::make_unique<ControllableActorSystem>(_registry, this)),
          animationSystem(std::make_unique<AnimationSystem>(_registry)),
          dialogueSystem(std::make_unique<DialogueSystem>(_registry, this)),
          healthBarSystem(std::make_unique<HealthBarSystem>(_registry, camera.get())),
          stateMachines(std::make_unique<StateMachines>(_registry, this)),
          abilitySystem(std::make_unique<PlayerAbilitySystem>(_registry, this)),
          combatSystem(std::make_unique<CombatSystem>(_registry))
    {
    }

    void GameData::Load()
    {
        serializer::Load(registry);
    }

    void GameData::Save() const
    {
        serializer::Save(*registry);
    }
} // namespace sage
