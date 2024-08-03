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
	
	enum class AttackElement
	{
		PHYSICAL,
		FIRE,
		ICE,
		LIGHTNING,
		POISON
	};
	
	struct AttackData
	{
		int damage = 0;
		AttackElement element = AttackElement::PHYSICAL;
	};
	
	struct CombatData
	{
		int hp;
		int maxHp;
		
	};

	struct CombatableActor
	{
		int hp = 100;
		entt::entity self;
		CombatableActorType actorType = CombatableActorType::WAVEMOB;
		bool dying = false;
		entt::entity target{};
		int attackRange = 5;
		float autoAttackTick = 0;
		float autoAttackTickThreshold = 1;
        
        std::vector<entt::delegate<void()>> dots;
        
		entt::sigh<void(entt::entity, entt::entity, AttackData)> onHit{}; // Self, attacker, damage
		entt::sigh<void(entt::entity)> onDeath{};
		entt::sigh<void(entt::entity, entt::entity)> onEnemyClicked{}; // Self, Clicked enemy
		entt::sigh<void(entt::entity)> onAttackCancelled{}; // Self
		entt::sigh<void(entt::entity, entt::entity)> onTargetDeath{}; // Self, target (that died)
        
		void EnemyClicked(entt::entity enemy);
		void AttackCancelled();
		void TargetDeath(entt::entity _target);
	};
} // sage
