#pragma once

#include <memory>
#include "units/UnitStateSystems.hpp"
#include "game/GameStateSystem.hpp"

namespace sage
{
	class GameData;
	class StateSystems
	{

	public:
		// Systems
		std::unique_ptr<UnitStateSystems> unitSystems;
		std::unique_ptr<GameStateSystem> gameSystem;
		void Update();
		void Draw3D();
		StateSystems(
				entt::registry* _registry,
				GameData* _gameData,
				Cursor* _cursor,
				TimerManager* _timerManager,
				ActorMovementSystem* _actorMovementSystem,
				CollisionSystem* _collisionSystem,
				ControllableActorSystem* _controllableActorSystem,
				NavigationGridSystem* _navigationGridSystem);
	};
} // sage