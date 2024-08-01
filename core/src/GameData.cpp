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
		collisionSystem(std::make_unique<CollisionSystem>(_registry)),
		navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry, collisionSystem.get())),
		actorMovementSystem(std::make_unique<ActorMovementSystem>(_registry,
			collisionSystem.get(),
			navigationGridSystem.get())),
		animationSystem(std::make_unique<AnimationSystem>(_registry)),
		timerManager(std::make_unique<TimerManager>())
	{
		userInput = std::make_unique<UserInput>(_keyMapping, settings);
		camera = std::make_unique<Camera>(userInput.get());
		particleSystem = std::make_unique<ParticleSystem>(camera->getRaylibCam());
		cursor = std::make_unique<Cursor>(registry,
			collisionSystem.get(),
			navigationGridSystem.get(),
			camera.get(),
			userInput.get());

		controllableActorSystem = std::make_unique<ControllableActorSystem>(_registry,
			cursor.get(),
			userInput.get(),
			navigationGridSystem.get(),
			actorMovementSystem.get());

		dialogueSystem = std::make_unique<DialogueSystem>(_registry,
			cursor.get(),
			camera.get(),
			settings,
			controllableActorSystem.get());
		healthBarSystem = std::make_unique<HealthBarSystem>(_registry, camera.get());
		stateSystems = std::make_unique<StateSystems>(_registry, cursor.get(), actorMovementSystem.get(), collisionSystem.get(),
			controllableActorSystem.get(), navigationGridSystem.get());
		abilitySystem = std::make_unique<AbilitySystem>(_registry, cursor.get(), userInput.get(), actorMovementSystem.get(),
			collisionSystem.get(), controllableActorSystem.get(), navigationGridSystem.get(), timerManager.get());
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
