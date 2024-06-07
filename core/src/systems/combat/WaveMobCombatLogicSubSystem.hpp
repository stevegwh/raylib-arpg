//
// Created by steve on 05/06/24.
//

#pragma once

#include "systems/StateMachineSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/CollisionSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{

struct WaveMobCombatLogicSubSystem
{
	entt::registry* registry;
    TransformSystem* transformSystem;
    CollisionSystem* collisionSystem;
    StateMachineSystem* stateMachineSystem;

    void Draw3D(entt::entity entity) const;
	void Update() const;
	void StartCombat(entt::entity entity);
	void CheckInCombat(entt::entity entity) const;
	void OnDeath(entt::entity entity);
	void AutoAttack(entt::entity entity) const;
	void OnHit(entt::entity entity, entt::entity attacker, float damage);
    void destroyEnemy(entt::entity entity);
	WaveMobCombatLogicSubSystem(entt::registry* _registry, StateMachineSystem* _stateMachineSystem , TransformSystem* _transformSystem, CollisionSystem* _collisionSystem);
};

} // sage
