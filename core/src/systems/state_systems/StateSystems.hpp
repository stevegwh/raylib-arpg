#pragma once

#include <memory>
#include "default/DefaultStateSystems.hpp"
#include "combat/CombatStateSystems.hpp"

namespace sage
{
	class StateSystems
	{

	public:
		// Systems
		std::unique_ptr<DefaultStateSystems> defaultSystems;
		std::unique_ptr<CombatStateSystems> combatSystems;
		void Update();
		void Draw3D();
		StateSystems(entt::registry* _registry, Cursor* _cursor, ActorMovementSystem* _actorMovementSystem, CollisionSystem* _collisionSystem,
			ControllableActorSystem* _controllableActorSystem, NavigationGridSystem* _navigationGridSystem);
	};
} // sage