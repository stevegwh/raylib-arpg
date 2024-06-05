//
// Created by Steve Wheeler on 03/06/2024.
//

#include "CombatSystem.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"

#include <iostream>

#include "raymath.h"

namespace sage
{

void CombatSystem::destroyEnemy(entt::entity entity)
{
    {
        auto& animation = registry->get<Animation>(entity);
        entt::sink sink { animation.onAnimationEnd };
        sink.disconnect<&CombatSystem::destroyEnemy>(this);
    }
    registry->destroy(entity);
}

void CombatSystem::onEnemyDead(entt::entity entity)
{
    auto& combatable = registry->get<Combatable>(entity);
    combatable.inCombat = false;
    auto& animation = registry->get<Animation>(entity);
    animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
    {
        entt::sink sink { animation.onAnimationEnd };
        sink.connect<&CombatSystem::destroyEnemy>(this);
    }
    {
        entt::sink sink{ cursor->onEnemyClick };
        sink.disconnect<&CombatSystem::onEnemyClick>(this);
    }
    auto& c = registry->get<Combatable>(entity);
    c.target = entt::null;
}

void CombatSystem::startCombat(entt::entity entity) // All this should do is set target, idle animation and combat flag
{
    // TODO: What is "entity"?
    {
        auto& player = registry->get<Transform>(actorMovementSystem->GetControlledActor());
        entt::sink sink { player.onFinishMovement };
        sink.disconnect<&CombatSystem::startCombat>(this);
    }
    auto& playerCombatable = registry->get<Combatable>(actorMovementSystem->GetControlledActor());
    playerCombatable.inCombat = true;
    
    // TODO: This should not be the way of starting combat with an enemy (should be on hit)
    auto& enemyCombatable = registry->get<Combatable>(playerCombatable.target);
    enemyCombatable.target = actorMovementSystem->GetControlledActor();
    enemyCombatable.inCombat = true;
}

void CombatSystem::onAutoAttackEnd(entt::entity entity)
{
}

void CombatSystem::CheckInCombat(entt::entity entity)
{
    // If the entity is not the target of any other combatable.

}

void CombatSystem::onEnemyClick(entt::entity entity)
{
    auto& combatable = registry->get<Combatable>(actorMovementSystem->GetControlledActor());
    combatable.target = entity;
    auto& player = registry->get<Transform>(actorMovementSystem->GetControlledActor());
    const auto& enemy = registry->get<Transform>(entity);
    actorMovementSystem->PathfindToLocation(actorMovementSystem->GetControlledActor(), enemy.position); // TODO: cannot be enemy position
    {
        entt::sink sink { player.onFinishMovement };
        sink.connect<&CombatSystem::startCombat>(this);
    }
}

void CombatSystem::Update()
{
    auto view = registry->view<Combatable, Transform>();
    for (auto& entity : view) 
    {
        auto& c = registry->get<Combatable>(entity);
        if (c.target == entt::null || !c.inCombat) continue;
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
}

CombatSystem::CombatSystem(entt::registry *_registry, Cursor *_cursor, ControllableActorMovementSystem* _actorMovementSystem) :
    BaseSystem<Combatable>(_registry), cursor(_cursor), actorMovementSystem(_actorMovementSystem)
{
    {
        entt::sink sink{ cursor->onEnemyClick };
        sink.connect<&CombatSystem::onEnemyClick>(this);  
    }
}
} // sage