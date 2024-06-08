//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

// Systems
#include "systems/StateMachineSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/WorldSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/ControllableActorMovementSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/dialogue/DialogueSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/state_systems/combat/CombatStateSystem.hpp"
#include "systems/state_systems/default/DefaultStateSystem.hpp"
#include "Settings.hpp"

#include "entt/entt.hpp"

#include <memory>

namespace sage
{
class GameData
{
    entt::registry* registry;
public:
    GameData(entt::registry* _registry, KeyMapping* _keyMapping, Settings* _settings);
    
    std::unique_ptr<UserInput> userInput;
    std::unique_ptr<Cursor> cursor;
    std::unique_ptr<Camera> camera;
    Settings* settings;
    
    std::unique_ptr<sage::StateMachineSystem> stateMachineSystem;
    std::unique_ptr<sage::CollisionSystem> collisionSystem;
    std::unique_ptr<sage::RenderSystem> renderSystem;
    std::unique_ptr<sage::TransformSystem> transformSystem;
    std::unique_ptr<sage::WorldSystem> worldSystem;
    std::unique_ptr<sage::NavigationGridSystem> navigationGridSystem;
    std::unique_ptr<sage::ControllableActorMovementSystem> actorMovementSystem;
    std::unique_ptr<sage::AnimationSystem> animationSystem;
    std::unique_ptr<sage::DialogueSystem> dialogueSystem;
    std::unique_ptr<sage::HealthBarSystem> healthBarSystem;
    std::unique_ptr<sage::CombatStateSystem> combatStateSystem;
    std::unique_ptr<sage::DefaultStateSystem> defaultStateSystem;

    void Load();
    void Save() const;

};

}

