//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
    class Cursor;
    class UserInput;
    class Camera;
    class LightSubSystem;
    struct Settings;
    struct KeyMapping;
    class GameUIEngine;

    class RenderSystem;
    class CollisionSystem;
    class NavigationGridSystem;
    class ActorMovementSystem;
    class ControllableActorSystem;
    class InventorySystem;
    class AnimationSystem;
    class DialogueSystem;
    class HealthBarSystem;
    class StateMachines;
    class AbilityFactory;
    class PlayerAbilitySystem;
    class CombatSystem;
    class TimerSystem;
    class EntityReflectionSignalRouter;

    // TODO: This should be owned by application, not "scene", and shouldn't be destroyed often.
    // TODO: Also, should rename to "GameSystems"
    class GameData
    {
      public:
        entt::registry* registry;
        Settings* settings;

        std::unique_ptr<UserInput> userInput;
        std::unique_ptr<Cursor> cursor;
        std::unique_ptr<Camera> camera;
        std::unique_ptr<LightSubSystem> lightSubSystem;
        std::unique_ptr<GameUIEngine> uiEngine;

        std::unique_ptr<RenderSystem> renderSystem;
        std::unique_ptr<CollisionSystem> collisionSystem;
        std::unique_ptr<NavigationGridSystem> navigationGridSystem;
        std::unique_ptr<ActorMovementSystem> actorMovementSystem;
        std::unique_ptr<ControllableActorSystem> controllableActorSystem;
        std::unique_ptr<AnimationSystem> animationSystem;
        std::unique_ptr<DialogueSystem> dialogueSystem;
        std::unique_ptr<HealthBarSystem> healthBarSystem;
        std::unique_ptr<StateMachines> stateMachines;
        std::unique_ptr<AbilityFactory> abilityRegistry;
        std::unique_ptr<PlayerAbilitySystem> playerAbilitySystem;
        std::unique_ptr<CombatSystem> combatSystem;
        std::unique_ptr<TimerSystem> timerSystem;
        std::unique_ptr<EntityReflectionSignalRouter> reflectionSignalRouter;
        std::unique_ptr<InventorySystem> inventorySystem;

        GameData(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings);
    };
} // namespace sage
