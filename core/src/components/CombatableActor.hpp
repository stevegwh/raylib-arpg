//
// Created by Steve Wheeler on 04/06/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
	enum class CombatableActorType
	{
		PLAYER,
		WAVEMOB
	};

	struct CombatableActor
	{
		entt::entity self;
		CombatableActorType actorType = CombatableActorType::WAVEMOB;
		bool dying = false;
		entt::entity target{};
		int attackRange = 5;
		float autoAttackTick = 0;
		float autoAttackTickThreshold = 1;
		entt::sigh<void(entt::entity, entt::entity, float)> onHit{};
		entt::sigh<void(entt::entity)> onDeath{};
		entt::sigh<void(entt::entity, entt::entity)> onEnemyClicked{}; // Self, Clicked enemy
		entt::sigh<void(entt::entity)> onAttackCancelled{}; // Self

		void EnemyClicked(entt::entity enemy)
		{
			target = enemy;
			onEnemyClicked.publish(self, enemy);
		}

		void AttackCancelled()
		{
			onAttackCancelled.publish(self);
		}
	};
} // sage
