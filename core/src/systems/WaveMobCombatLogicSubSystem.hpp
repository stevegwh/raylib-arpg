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
	void Update(entt::entity entity);
	void StartCombat(entt::entity entity);
	void CheckInCombat(entt::entity entity);
	void OnDeath(entt::entity entity);
	void AutoAttack(entt::entity entity);
	void OnHit(entt::entity entity, entt::entity attacker);
	explicit WaveMobCombatLogicSubSystem(entt::registry* _registry);
};

} // sage
