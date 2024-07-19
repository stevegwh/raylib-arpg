//
// Created by Steve Wheeler on 03/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "Cursor.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "PlayerCombatStateSystem.hpp"
#include "WaveMobCombatStateSystem.hpp"


#include <vector>

namespace sage
{
	class ActorMovementSystem; // forward dec
	class CollisionSystem; // forward dec
	class CombatStateSystems
	{
		entt::registry* registry;
		Cursor* cursor;
		ControllableActorSystem* actorMovementSystem;
		std::vector<StateMachineSystem*> systems;

	public:
		std::unique_ptr<PlayerCombatStateSystem> playerCombatLogicSubSystem;
		std::unique_ptr<WaveMobCombatStateSystem> waveMobCombatLogicSubSystem;
		CombatStateSystems(entt::registry* _registry,
		                  Cursor* _cursor,
		                  ControllableActorSystem* _actorMovementSystem,
		                  ActorMovementSystem* _transformSystem,
		                  CollisionSystem* _collisionSystem,
		                  NavigationGridSystem* _navigationGridSystem);
		void Update();
		void Draw3D();
	};
} // sage
