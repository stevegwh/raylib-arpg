//
// Created by Steve Wheeler on 08/06/2024.
//

#pragma once

#include "systems/ActorMovementSystem.hpp"
#include "default/WaveMobDefaultStateSystem.hpp"
#include "default/PlayerDefaultStateSystem.hpp"
#include "combat/PlayerCombatStateSystem.hpp"
#include "combat/WaveMobCombatStateSystem.hpp"
#include "Cursor.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "entt/entt.hpp"

#include <vector>
#include <memory>

namespace sage
{
	class CollisionSystem; // forward dec
	class NavigationGridSystem; // forward dec
	class TimerManager; // forward dec

	class UnitStateSystems
	{
	private:
		entt::registry* registry;
		Cursor* cursor;
		ControllableActorSystem* controllableActorSystem;
		std::vector<BaseSystem*> systems;

	public:
		std::unique_ptr<WaveMobDefaultStateSystem> waveMobDefaultSubSystem;
		std::unique_ptr<PlayerDefaultStateSystem> playerDefaultSubSystem;
		std::unique_ptr<PlayerCombatStateSystem> playerCombatLogicSubSystem;
		std::unique_ptr<WaveMobCombatStateSystem> waveMobCombatLogicSubSystem;

		UnitStateSystems(
				entt::registry* _registry,
				Cursor* _cursor,
				ControllableActorSystem* _controllableActorSystem,
				ActorMovementSystem* _actorMovementSystem,
				CollisionSystem* _collisionSystem,
				NavigationGridSystem* _navigationGridSystem,
				TimerManager* _timerManager);

		void Update();
		void Draw3D();
	};
} // sage