//
// Created by steve on 05/06/24.
//

#pragma once

#include "systems/state_systems/StateMachineSystem.hpp"
#include "components/states/StateEnemyCombat.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
	class WaveMobCombatStateSystem : public StateMachineSystem<WaveMobCombatStateSystem, StateEnemyCombat>
	{
		NavigationGridSystem* navigationGridSystem;
		ActorMovementSystem* actorMovementSystem;
		CollisionSystem* collisionSystem;

		void startCombat(entt::entity entity);
		[[nodiscard]] bool checkInCombat(entt::entity entity);
		void onDeath(entt::entity entity);
		void autoAttack(entt::entity entity) const;
		void destroyEnemy(entt::entity entity);
		void onTargetOutOfRange(entt::entity entity, Vector3& normDirection, float distance) const;
	public:
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;
		void OnHit(entt::entity entity, entt::entity attacker, float damage);
		void Draw3D() override;
		void Update() override;
		WaveMobCombatStateSystem(entt::registry* _registry,
		                            ActorMovementSystem* _actorMovementSystem,
		                            CollisionSystem* _collisionSystem,
		                            NavigationGridSystem* _navigationGridSystem);
	};
} // sage
