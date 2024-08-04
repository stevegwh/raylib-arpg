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

		void setTarget(entt::entity self);
		void startCombat(entt::entity self);
		[[nodiscard]] bool checkInCombat(entt::entity self);
		void onDeath(entt::entity self);
		void onTargetDeath(entt::entity self, entt::entity target);
		void onAttackCancel(entt::entity self);
		void autoAttack(entt::entity self) const;
	public:
		void OnEnemyClick(entt::entity self, entt::entity target);
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
