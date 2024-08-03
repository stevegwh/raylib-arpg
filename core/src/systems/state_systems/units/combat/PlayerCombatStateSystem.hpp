//
// Created by Steve on 05/06/24.
//

#pragma once

#include "systems/state_systems/StateMachineSystem.hpp"
#include "components/states/PlayerStates.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "components/CombatableActor.hpp"

#include "entt/entt.hpp"


namespace sage
{
	class PlayerCombatStateSystem : public StateMachineSystem<PlayerCombatStateSystem, StatePlayerCombat>
	{
		ControllableActorSystem* controllableActorSystem;

		void startCombat(entt::entity entity);
		[[nodiscard]] bool checkInCombat(entt::entity entity);
		void onDeath(entt::entity entity);
		void onTargetDeath(entt::entity actor, entt::entity target);
		void onAttackCancel(entt::entity entity);
		void autoAttack(entt::entity entity) const;
	public:
		void OnEnemyClick(entt::entity actor, entt::entity target);
		void Update() override;
		void Draw3D() override;
		void OnHit(AttackData attackData);
		void Enable();
		void Disable();
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;

		PlayerCombatStateSystem(entt::registry* _registry,
		                           ControllableActorSystem* _controllableActorSystem);
	};
} // sage
