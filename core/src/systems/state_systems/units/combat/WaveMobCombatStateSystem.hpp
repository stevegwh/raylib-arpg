//
// Created by steve on 05/06/24.
//

#pragma once

#include "systems/state_systems/StateMachineSystem.hpp"
#include "components/states/EnemyStates.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "components/CombatableActor.hpp"
#include "TimerManager.hpp"

#include "entt/entt.hpp"

#include <memory>


namespace sage
{
	class WaveMobCombatStateSystem : public StateMachineSystem<WaveMobCombatStateSystem, StateEnemyCombat>
	{
		NavigationGridSystem* navigationGridSystem;
		ActorMovementSystem* actorMovementSystem;
		CollisionSystem* collisionSystem;
		bool isTargetInLineOfSight(entt::entity self, Vector3& normDirection, float distance) const;
		void startCombat(entt::entity self);
		[[nodiscard]] bool checkInCombat(entt::entity self);
		void onDeath(entt::entity self);
		void tryAutoAttack(entt::entity self) const;
		void destroyEnemy(entt::entity self);
		void onTargetOutOfRange(entt::entity entity, Vector3& normDirection, float distance) const;
	public:
		void OnStateEnter(entt::entity self) override;
		void OnStateExit(entt::entity self) override;
		void OnHit(AttackData attackData);
		void Draw3D() override;
		void Update() override;
		WaveMobCombatStateSystem(entt::registry* _registry,
		                            ActorMovementSystem* _actorMovementSystem,
		                            CollisionSystem* _collisionSystem,
		                            NavigationGridSystem* _navigationGridSystem,
									TimerManager* _timerManager);
	};
} // sage
