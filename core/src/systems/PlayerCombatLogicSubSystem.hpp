//
// Created by Steve on 05/06/24.
//

#pragma once

#include "systems/CombatLogicSubSystem.hpp"

namespace sage
{

struct PlayerCombatLogicSubSystem : public CombatLogicSubSystem
{
	void Update(entt::entity entity) override;
	void StartCombat(entt::entity entity) override;
	void CheckInCombat(entt::entity entity) override;
	void OnDeath(entt::entity entity) override;
	void AutoAttack(entt::entity entity) override;
	void OnHit(entt::entity entity, entt::entity attacker) override;
};

} // sage
