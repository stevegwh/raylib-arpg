#include "StateSystems.hpp"

namespace sage
{
	void StateSystems::Update()
	{
		defaultSystems->Update();
		combatSystems->Update();
	}

	void StateSystems::Draw3D()
	{
		// defaultStateSystems->Draw3D();
		combatSystems->Draw3D();
	}

	StateSystems::StateSystems(entt::registry* _registry, Cursor* _cursor, ActorMovementSystem* _actorMovementSystem, CollisionSystem* _collisionSystem,
			ControllableActorSystem* _controllableActorSystem, NavigationGridSystem* _navigationGridSystem) :
		defaultSystems(std::make_unique<DefaultStateSystems>(_registry, _actorMovementSystem))
	{
		
		combatSystems = std::make_unique<CombatStateSystems>(_registry,
		                                                        _cursor,
		                                                        _controllableActorSystem,
		                                                        _actorMovementSystem,
		                                                        _collisionSystem,
		                                                        _navigationGridSystem);
	}
}//sage