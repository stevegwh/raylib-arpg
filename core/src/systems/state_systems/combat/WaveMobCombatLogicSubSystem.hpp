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

class WaveMobCombatLogicSubSystem
{
	entt::registry* registry;
    TransformSystem* transformSystem;
    CollisionSystem* collisionSystem;
    StateMachineSystem* stateMachineSystem;

    void StartCombat(entt::entity entity);
    [[nodiscard]] bool CheckInCombat(entt::entity entity) const;
    void OnDeath(entt::entity entity);
    void AutoAttack(entt::entity entity) const;
    void destroyEnemy(entt::entity entity);
public:
    void OnHit(entt::entity entity, entt::entity attacker, float damage);
    void Draw3D(entt::entity entity) const;
	void Update() const;
	WaveMobCombatLogicSubSystem(entt::registry* _registry, StateMachineSystem* _stateMachineSystem, TransformSystem* _transformSystem, CollisionSystem* _collisionSystem);
};

} // sage
