//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

#include "Camera.hpp"
#include "Cursor.hpp"
#include "UserInput.hpp"

// Systems
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

#include "Settings.hpp"

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
    class GameData
    {

      public:
        entt::registry* registry;
        GameData(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings);

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
        std::unique_ptr<PlayerAbilitySystem> abilitySystem;
        std::unique_ptr<CombatSystem> combatSystem;

        void Load();
        void Save() const;
    };
} // namespace sage
