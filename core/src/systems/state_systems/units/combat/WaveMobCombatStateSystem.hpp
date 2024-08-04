//
// Created by steve on 05/06/24.
//

#pragma once

#include "systems/state_systems/StateMachineSystem.hpp"
#include "components/states/EnemyStates.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"

#include "entt/entt.hpp"
#include "components/CombatableActor.hpp"

namespace sage
{
	class WaveMobCombatStateSystem : public StateMachineSystem<WaveMobCombatStateSystem, StateEnemyCombat>
	{
		NavigationGridSystem* navigationGridSystem;
		ActorMovementSystem* actorMovementSystem;
		CollisionSystem* collisionSystem;

		void startCombat(entt::entity self);
		[[nodiscard]] bool checkInCombat(entt::entity self);
		void onDeath(entt::entity self);
		void autoAttack(entt::entity self) const;
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
		                            NavigationGridSystem* _navigationGridSystem);
	};
} // sage
