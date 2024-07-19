//
// Created by Steve on 05/06/24.
//

#pragma once

#include "systems/StateMachineSystem.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
	class PlayerCombatLogicSubSystem : public StateMachineSystem
	{
		entt::entity playerEntity;
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
		void Draw3D(entt::entity entity) override;
		void OnHit(entt::entity entity, entt::entity attacker);
		void Enable();
		void Disable();
		void OnStateEnter(entt::entity entity) override;
		void OnStateExit(entt::entity entity) override;

		PlayerCombatLogicSubSystem(entt::registry* _registry,
		                           ControllableActorSystem* _controllableActorSystem);
	};
} // sage
