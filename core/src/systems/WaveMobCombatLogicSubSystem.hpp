//
// Created by steve on 05/06/24.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{

struct WaveMobCombatLogicSubSystem
{
	entt::registry* registry;
	void Update(entt::entity entity) const;
	void StartCombat(entt::entity entity);
	void CheckInCombat(entt::entity entity) const;
	void OnDeath(entt::entity entity);
	void AutoAttack(entt::entity entity) const;
	void OnHit(entt::entity entity, entt::entity attacker, float damage);
    void destroyEnemy(entt::entity entity);
	explicit WaveMobCombatLogicSubSystem(entt::registry* _registry);
};

} // sage
