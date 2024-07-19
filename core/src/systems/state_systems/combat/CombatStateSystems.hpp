//
// Created by Steve Wheeler on 03/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "Cursor.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "PlayerCombatLogicSubSystem.hpp"
#include "WaveMobCombatLogicSubSystem.hpp"

namespace sage
{
	class ActorMovementSystem; // forward dec
	class CollisionSystem; // forward dec
	class CombatStateSystems
	{
		entt::registry* registry;
		Cursor* cursor;
		ControllableActorSystem* actorMovementSystem;

	public:
		std::unique_ptr<PlayerCombatLogicSubSystem> playerCombatLogicSubSystem;
		std::unique_ptr<WaveMobCombatLogicSubSystem> waveMobCombatLogicSubSystem;
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
