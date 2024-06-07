//
// Created by steve on 05/06/24.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{

struct CombatLogicSubSystem
{
	virtual void Update(entt::entity entity) = 0;
	virtual void CheckInCombat(entt::entity entity) = 0;
	virtual void StartCombat(entt::entity entity) = 0;
	virtual void OnDeath(entt::entity entity) = 0;
	virtual void AutoAttack(entt::entity entity) = 0;
	virtual void OnHit(entt::entity entity, entt::entity attacker) = 0;
};

} // sage
