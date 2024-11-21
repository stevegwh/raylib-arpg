//
// Created by Steve Wheeler on 27/03/2024.
//

#include "GameData.hpp"
#include <Serializer.hpp>

#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameUiEngine.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"

// Systems
#include "AbilityFactory.hpp"
#include "EntityReflectionSignalRouter.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/DialogSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/LightSubSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/states/StateMachines.hpp"
#include "systems/TimerSystem.hpp"
#include "systems/UberShaderSystem.hpp"

namespace sage
{
    GameData::GameData(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings)
        : registry(_registry),
          settings(_settings),
          userInput(std::make_unique<UserInput>(_keyMapping, _settings)),
          cursor(std::make_unique<Cursor>(_registry, this)),
          camera(std::make_unique<Camera>(_registry, userInput.get(), this)),
          lightSubSystem(std::make_unique<LightSubSystem>(_registry, camera.get())),
          uiEngine(std::make_unique<GameUIEngine>(_registry, this)),
          renderSystem(std::make_unique<RenderSystem>(_registry)),
          collisionSystem(std::make_unique<CollisionSystem>(_registry)),
          navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry, collisionSystem.get())),
          actorMovementSystem(
              std::make_unique<ActorMovementSystem>(_registry, collisionSystem.get(), navigationGridSystem.get())),
          controllableActorSystem(std::make_unique<ControllableActorSystem>(_registry, this)),
          animationSystem(std::make_unique<AnimationSystem>(_registry)),
          dialogSystem(std::make_unique<DialogSystem>(_registry, this)),
          healthBarSystem(std::make_unique<HealthBarSystem>(_registry, camera.get())),
          stateMachines(std::make_unique<StateMachines>(_registry, this)),
          abilityRegistry(std::make_unique<AbilityFactory>(_registry, this)),
          playerAbilitySystem(std::make_unique<PlayerAbilitySystem>(_registry, this)),
          combatSystem(std::make_unique<CombatSystem>(_registry)),
          timerSystem(std::make_unique<TimerSystem>(_registry)),
          reflectionSignalRouter(std::make_unique<EntityReflectionSignalRouter>()),
          inventorySystem(std::make_unique<InventorySystem>(_registry, this)),
          partySystem(std::make_unique<PartySystem>(_registry, this)),
          equipmentSystem(std::make_unique<EquipmentSystem>(_registry, this)),
          uberShaderSystem(std::make_unique<UberShaderSystem>(_registry, this))
    {
        // TODO: Move GameData out of Scene and into Application
        // Minus lights and timers, I'm not sure if anything would suffer from this
    }
} // namespace sage
