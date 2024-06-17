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
    auto& c = registry->get<CombatableActor>(entity);
    auto& t = registry->get<Transform>(entity);
    auto& collideable = registry->get<Collideable>(entity);
    auto& animation = registry->get<Animation>(entity);

    auto& enemyPos = registry->get<Transform>(c.target).position;
    Vector3 direction = Vector3Subtract(enemyPos, t.position);
    float distance = Vector3Length(direction);
    Vector3 normDirection = Vector3Normalize(direction);

    if (distance >= c.attackRange)
    {
        animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
        Ray ray;
        ray.position = t.position;
        ray.direction = Vector3Scale(normDirection, distance);
        ray.position.y = 0.5f;
        ray.direction.y = 0.5f;
        t.movementDirectionDebugLine = ray;
        auto collisions = collisionSystem->GetCollisionsWithRay(entity, ray, collideable.collisionLayer);
        
        if (!collisions.empty() && collisions.at(0).collisionLayer != CollisionLayer::PLAYER)
        {

            // Lost line of sight, out of combat
            transformSystem->CancelMovement(entity);
            c.target = entt::null;
            t.movementDirectionDebugLine = {};
            return;
        }
        // TODO: PathfindToLocation also calls PruneMoveCommands which triggers onMovementCancel
        transformSystem->PathfindToLocation(entity, {enemyPos});
        return;
    }

    float angle = atan2f(direction.x, direction.z) * RAD2DEG;
    t.rotation.y = angle;
    c.autoAttackTick = 0;
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
    transformSystem->CancelMovement(attacker); 
    transformSystem->PruneMoveCommands(entity);
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
                                                         ActorMovementSystem* _transformSystem,
                                                         CollisionSystem* _collisionSystem) :
                                                         registry(_registry),
                                                         stateMachineSystem(_stateMachineSystem),
                                                         transformSystem(_transformSystem),
                                                         collisionSystem(_collisionSystem)
{}
} // sage