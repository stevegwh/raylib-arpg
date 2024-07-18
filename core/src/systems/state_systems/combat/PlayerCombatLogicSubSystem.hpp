//
// Created by Steve on 05/06/24.
//

#pragma once

#include "systems/StateMachineSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
	struct PlayerCombatLogicSubSystem
	{
		entt::registry* registry;
		entt::entity playerEntity;
		StateMachineSystem* stateMachineSystem;
		ControllableActorSystem* controllableActorSystem;

		// TODO: Move all non-combat related logic to the default system (i.e., enemy clicked etc)

		void onEnemyClick(entt::entity actor, entt::entity target);

		void Update() const;
		void StartCombat(entt::entity entity);
		[[nodiscard]] bool CheckInCombat(entt::entity entity) const;
		void OnDeath(entt::entity entity);
		void OnTargetDeath(entt::entity entity);
		void OnAttackCancel(entt::entity entity);
		void AutoAttack(entt::entity entity) const;
		void OnHit(entt::entity entity, entt::entity attacker);
		void Enable();
		void Disable();
		void OnStateAdded(entt::entity entity) const;
		void OnStateRemoved(entt::entity entity) const;

		PlayerCombatLogicSubSystem(entt::registry* _registry,
		                           StateMachineSystem* _stateMachineSystem,
		                           ControllableActorSystem* _controllableActorSystem);
	};
} // sage
