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
    struct Settings;
    struct KeyMapping;

    class RenderSystem;
    class CollisionSystem;
    class NavigationGridSystem;
    class ActorMovementSystem;
    class ControllableActorSystem;
    class AnimationSystem;
    class DialogueSystem;
    class HealthBarSystem;
    class StateMachines;
    class AbilityFactory;
    class PlayerAbilitySystem;
    class CombatSystem;
    class TimerSystem;
    class EntityReflectionSignalRouter;
    class LightSubSystem;

    class GameData
    {
      public:
        entt::registry* registry;

        std::unique_ptr<UserInput> userInput;
        std::unique_ptr<Cursor> cursor;
        std::unique_ptr<Camera> camera;
        Settings* settings;

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
        std::unique_ptr<EntityReflectionSignalRouter> signalReflectionManager;
        LightSubSystem* lightSubSystem; // Owned by scene (TODO: Why?)

        void Load();
        void Save() const;
        GameData(
            entt::registry* _registry,
            KeyMapping* _keyMapping,
            Settings* _settings,
            LightSubSystem* _lightSubSystem);
    };
} // namespace sage
