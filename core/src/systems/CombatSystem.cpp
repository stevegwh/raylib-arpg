//
// Created by Steve Wheeler on 03/06/2024.
//

#include "CombatSystem.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"

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
    auto& animation = registry->get<Animation>(entity);
    animation.ChangeAnimation(0, true); // TODO: Magic number for changing animations
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

void CombatSystem::startCombat(entt::entity entity)
{
    auto& player = registry->get<Transform>(actorMovementSystem->GetControlledActor());
    {
        entt::sink sink { player.onFinishMovement };
        sink.disconnect<&CombatSystem::startCombat>(this);
    }
    auto& combatable = registry->get<Combatable>(actorMovementSystem->GetControlledActor());
    auto& animation = registry->get<Animation>(combatable.target);
    animation.ChangeAnimation(1); // TODO: Magic number for changing animations
    auto& healthbar = registry->get<HealthBar>(combatable.target);
    healthbar.Decrement(combatable.target, 10); // TODO: tmp

    if (healthbar.hp <= 0)
    {
        onEnemyDead(combatable.target);
    }
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
        if (c.target == entt::null) continue;
        auto& t = registry->get<Transform>(entity);
        // Turn to look at target
        // Progress tick
        // Autoattack
        // Calculate rotation angle based on direction
        auto& enemyPos = registry->get<Transform>(c.target).position;
        Vector3 direction = Vector3Subtract(enemyPos, t.position);
        float angle = atan2f(direction.x, direction.z) * RAD2DEG;
        t.rotation.y = angle;
    }
}

CombatSystem::CombatSystem(entt::registry *_registry, Cursor *_cursor, ActorMovementSystem* _actorMovementSystem) :
    BaseSystem<Combatable>(_registry), cursor(_cursor), actorMovementSystem(_actorMovementSystem)
{
    {
        entt::sink sink{ cursor->onEnemyClick };
        sink.connect<&CombatSystem::onEnemyClick>(this);  
    }
}
} // sage