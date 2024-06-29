//
// Created by steve on 05/06/24.
//

#include "raylib.h"
#include "raymath.h"

#include "components/states/StateEnemyCombat.hpp"
#include "components/states/StateEnemyDefault.hpp"
#include "WaveMobCombatLogicSubSystem.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"
#include "components/Transform.hpp"
#include "components/HealthBar.hpp"
#include "components/states/EnemyStateComponents.hpp"

namespace sage
{



void WaveMobCombatLogicSubSystem::Update() const
{
    auto view = registry->view<CombatableActor, StateEnemyCombat>();

    for (const auto& entity: view)
    {
        auto& c = registry->get<CombatableActor>(entity);
        if (!CheckInCombat(entity)) continue;
        if (c.dying) continue;

        // Player is out of combat if no enemy is targetting them?
        if (c.autoAttackTick >= c.autoAttackTickThreshold) // Maybe can count time since last autoattack to time out combat?
        {
            AutoAttack(entity);
        }
        else
        {
            c.autoAttackTick += GetFrameTime();
        }
    }
}

bool WaveMobCombatLogicSubSystem::CheckInCombat(entt::entity entity) const
{
    // If the entity is not the target of any other combatable.
	// If no current target
	// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
	auto& combatable = registry->get<CombatableActor>(entity);
	if (combatable.target == entt::null && !combatable.dying)
	{
        stateMachineSystem->ChangeState<StateEnemyDefault, StateComponents>(entity);
        return false;
	}
    return true;
}

void WaveMobCombatLogicSubSystem::destroyEnemy(entt::entity entity)
{
    
    navigationGridSystem->MarkSquareOccupied(registry->get<Collideable>(entity).worldBoundingBox, false);
    {
        auto& animation = registry->get<Animation>(entity);
        entt::sink sink { animation.onAnimationEnd };
        sink.disconnect<&WaveMobCombatLogicSubSystem::destroyEnemy>(this);
    }
    registry->destroy(entity);
}

void WaveMobCombatLogicSubSystem::OnDeath(entt::entity entity)
{
    auto& combatable = registry->get<CombatableActor>(entity);
    combatable.onDeath.publish(entity);
	combatable.target = entt::null;
    combatable.dying = true;
	
    {
        entt::sink sink { combatable.onHit };
        sink.disconnect<&WaveMobCombatLogicSubSystem::OnHit>(this);
    }
	
    auto& animation = registry->get<Animation>(entity);
    animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
    {
        entt::sink sink { animation.onAnimationEnd };
        sink.connect<&WaveMobCombatLogicSubSystem::destroyEnemy>(this);
    }
}

void WaveMobCombatLogicSubSystem::AutoAttack(entt::entity entity) const
{
    auto& combatableActor = registry->get<CombatableActor>(entity);
    auto& actorTrans = registry->get<Transform>(entity);
    auto& collideable = registry->get<Collideable>(entity);
    auto& animation = registry->get<Animation>(entity);

    auto& target = registry->get<Transform>(combatableActor.target).position;

	Vector3 direction = Vector3Subtract(target, actorTrans.position);
    float distance = Vector3Distance(actorTrans.position, target);
    Vector3 normDirection = Vector3Normalize(direction);

    if (distance >= 15)
    {
        animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
        Ray ray;
        ray.position = actorTrans.position;
        ray.direction = Vector3Scale(normDirection, distance);
        ray.position.y = 0.5f;
        ray.direction.y = 0.5f;
        actorTrans.movementDirectionDebugLine = ray;
        auto collisions = collisionSystem->GetCollisionsWithRay(entity, ray, collideable.collisionLayer);

        if (!collisions.empty() && collisions.at(0).collisionLayer != CollisionLayer::PLAYER)
        {

            // Lost line of sight, out of combat
            actorMovementSystem->CancelMovement(entity);
            combatableActor.target = entt::null;
            actorTrans.movementDirectionDebugLine = {};
            return;
        }
        const auto& moveableActor = registry->get<MoveableActor>(entity);
        if (!moveableActor.destination.has_value())
        {
            actorMovementSystem->PathfindToLocation(entity, target, true);
        }
        return;
    }

    float angle = atan2f(direction.x, direction.z) * RAD2DEG;
    actorTrans.rotation.y = angle;
    combatableActor.autoAttackTick = 0;
    animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
}

void WaveMobCombatLogicSubSystem::Draw3D(entt::entity entity) const
{
    auto& t = registry->get<Transform>(entity);
    //DrawLine3D(t.movementDirectionDebugLine.position, t.movementDirectionDebugLine.direction, RED);
}

void WaveMobCombatLogicSubSystem::StartCombat(entt::entity entity)
{

}

void WaveMobCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker, float damage)
{
    // TODO: Would prefer this to be handled in a changing state event like OnExit OnEnter
    actorMovementSystem->CancelMovement(attacker); 
    actorMovementSystem->CancelMovement(entity);
    // -----
    stateMachineSystem->ChangeState<StateEnemyCombat, StateComponents>(entity);
    // Aggro when player hits
    auto& c = registry->get<CombatableActor>(entity);
    c.target = attacker;
	
	auto& healthbar = registry->get<HealthBar>(entity);
	healthbar.Decrement(entity, damage);
	if (healthbar.hp <= 0)
	{
		c.target = entt::null;
		OnDeath(entity);
	}
}

WaveMobCombatLogicSubSystem::WaveMobCombatLogicSubSystem(entt::registry *_registry,
    StateMachineSystem* _stateMachineSystem,
    ActorMovementSystem* _actorMovementSystem,
    CollisionSystem* _collisionSystem,
    NavigationGridSystem* _navigationGridSystem) :
	registry(_registry),
	navigationGridSystem(_navigationGridSystem),
	actorMovementSystem(_actorMovementSystem),
	collisionSystem(_collisionSystem),
	stateMachineSystem(_stateMachineSystem)
{}
} // sage