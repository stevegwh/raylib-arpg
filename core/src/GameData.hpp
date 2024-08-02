//
// Created by Steve Wheeler on 27/03/2024.
//

#pragma once

// Systems
#include "systems/RenderSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/WorldSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/dialogue/DialogueSystem.hpp"
#include "systems/HealthBarSystem.hpp"
#include "systems/state_systems/StateSystems.hpp"
#include "systems/AbilitySystem.hpp"

#include "Settings.hpp"

#include "entt/entt.hpp"
#include "TimerManager.hpp"

#include <memory>
#include <string>

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
		std::unique_ptr<TimerManager> timerManager;
		Settings* settings;

		std::unique_ptr<RenderSystem> renderSystem;
		std::unique_ptr<CollisionSystem> collisionSystem;
		std::unique_ptr<NavigationGridSystem> navigationGridSystem;
		std::unique_ptr<ActorMovementSystem> actorMovementSystem;
		std::unique_ptr<WorldSystem> worldSystem;
		std::unique_ptr<ControllableActorSystem> controllableActorSystem;
		std::unique_ptr<AnimationSystem> animationSystem;
		std::unique_ptr<DialogueSystem> dialogueSystem;
		std::unique_ptr<HealthBarSystem> healthBarSystem;
		std::unique_ptr<StateSystems> stateSystems;
        std::unique_ptr<AbilitySystem> abilitySystem;
        
		void Load();
		void Save() const;
	};
}
