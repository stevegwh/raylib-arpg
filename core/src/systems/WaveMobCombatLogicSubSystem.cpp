//
// Created by steve on 05/06/24.
//

#include "WaveMobCombatLogicSubSystem.hpp"
#include "../components/CombatableActor.hpp"
#include "../components/Animation.hpp"

#include "raylib.h"
#include "raymath.h"
	

namespace sage
{
void WaveMobCombatLogicSubSystem::Update(entt::entity entity)
{
	CheckInCombat(entity);
	auto& c = registry->get<CombatableActor>(entity);
	if (c.target == entt::null || !c.inCombat) return;
	auto& t = registry->get<Transform>(entity);
	// Move to target
	// Progress tick
	// Turn to look at target
	// Autoattack

	// Player is out of combat if no enemy is targetting them?
	if (c.autoAttackTick >= c.autoAttackTickThreshold) // Maybe can count time since last autoattack to time out combat?
	{
		auto& enemyPos = registry->get<Transform>(c.target).position;
		Vector3 direction = Vector3Subtract(enemyPos, t.position);
		float angle = atan2f(direction.x, direction.z) * RAD2DEG;
		t.rotation.y = angle;
		c.autoAttackTick = 0;

		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);

		if (registry->all_of<HealthBar>(c.target))
		{
			auto& healthbar = registry->get<HealthBar>(c.target);
			healthbar.Decrement(c.target, 10); // TODO: tmp
			if (healthbar.hp <= 0)
			{
				onEnemyDead(c.target);
				c.target = entt::null;
			}
		}
	}
	else
	{
		c.autoAttackTick += GetFrameTime();
	}
}
void WaveMobCombatLogicSubSystem::CheckInCombat(entt::entity entity)
{

}
void WaveMobCombatLogicSubSystem::OnDeath(entt::entity entity)
{

}
void WaveMobCombatLogicSubSystem::AutoAttack(entt::entity entity)
{

}
void WaveMobCombatLogicSubSystem::StartCombat(entt::entity entity)
{

}
void WaveMobCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker)
{

}
WaveMobCombatLogicSubSystem::WaveMobCombatLogicSubSystem(entt::registry *_registry) :
	registry(_registry)
{

}
} // sage