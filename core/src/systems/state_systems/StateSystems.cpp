#include "StateSystems.hpp"

namespace sage
{
	void StateSystems::Update()
	{
		unitSystems->Update();
		gameSystem->Update();
	}

	void StateSystems::Draw3D()
	{
		unitSystems->Draw3D();
		// gameSystem->Draw3D();
	}

	StateSystems::StateSystems(
			entt::registry* _registry,
			Cursor* _cursor,
			TimerManager* _timerManager,
			ActorMovementSystem* _actorMovementSystem,
			CollisionSystem* _collisionSystem,
			ControllableActorSystem* _controllableActorSystem,
			NavigationGridSystem* _navigationGridSystem)
	{

		unitSystems = std::make_unique<UnitStateSystems>(
				_registry,
				_cursor,
				_controllableActorSystem,
				_actorMovementSystem,
				_collisionSystem,
				_navigationGridSystem);

		gameSystem = std::make_unique<GameStateSystem>(_registry, _timerManager);
	}
}//sage