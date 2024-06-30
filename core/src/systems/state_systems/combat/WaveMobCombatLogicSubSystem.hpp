//
// Created by steve on 05/06/24.
//

#pragma once

#include "systems/StateMachineSystem.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/CollisionSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{

class WaveMobCombatLogicSubSystem
{
	entt::registry* registry;
	NavigationGridSystem* navigationGridSystem;
    ActorMovementSystem* actorMovementSystem;
    CollisionSystem* collisionSystem;
    StateMachineSystem* stateMachineSystem;

    void StartCombat(entt::entity entity);
    [[nodiscard]] bool CheckInCombat(entt::entity entity) const;
    void OnDeath(entt::entity entity);
    void AutoAttack(entt::entity entity) const;
    void destroyEnemy(entt::entity entity);
    void onTargetOutOfRange(entt::entity entity, Vector3& normDirection, float distance) const;
public:
	void OnComponentEnabled(entt::entity entity) const;
	void OnComponentDisabled(entt::entity entity) const;
    void OnHit(entt::entity entity, entt::entity attacker, float damage);
    void Draw3D(entt::entity entity) const;
	void Update() const;
	WaveMobCombatLogicSubSystem(entt::registry* _registry, 
        StateMachineSystem* _stateMachineSystem, 
        ActorMovementSystem* _actorMovementSystem, 
        CollisionSystem* _collisionSystem,
        NavigationGridSystem* _navigationGridSystem);
};

} // sage
