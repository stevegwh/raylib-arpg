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
		stateMachineSystem(std::make_unique<StateMachineSystem>(_registry)),
		renderSystem(std::make_unique<RenderSystem>(_registry)),
		collisionSystem(std::make_unique<CollisionSystem>(_registry)),
		navigationGridSystem(std::make_unique<NavigationGridSystem>(_registry, collisionSystem.get())),
		actorMovementSystem(std::make_unique<ActorMovementSystem>(_registry,
		                                                          collisionSystem.get(),
		                                                          navigationGridSystem.get())),
		animationSystem(std::make_unique<AnimationSystem>(_registry)),
		defaultStateSystem(std::make_unique<DefaultStateSystem>(_registry,
		                                                        stateMachineSystem.get(),
		                                                        actorMovementSystem.get()))
	{
		userInput = std::make_unique<UserInput>(_keyMapping, settings);
		camera = std::make_unique<Camera>(userInput.get());
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
		// TODO: Don't like this here. (Put it in cursor constructor)
		{
			entt::sink sink{controllableActorSystem->onControlledActorChange};
			sink.connect<&Cursor::OnControlledActorChange>(*cursor);
		}
		dialogueSystem = std::make_unique<DialogueSystem>(_registry,
		                                                  cursor.get(),
		                                                  camera.get(),
		                                                  settings,
		                                                  controllableActorSystem.get());
		healthBarSystem = std::make_unique<HealthBarSystem>(_registry, camera.get());
		combatStateSystem = std::make_unique<CombatStateSystem>(_registry,
		                                                        cursor.get(),
		                                                        stateMachineSystem.get(),
		                                                        controllableActorSystem.get(),
		                                                        actorMovementSystem.get(),
		                                                        collisionSystem.get(),
		                                                        navigationGridSystem.get());
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
