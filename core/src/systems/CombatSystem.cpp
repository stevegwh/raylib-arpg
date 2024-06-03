//
// Created by Steve Wheeler on 03/06/2024.
//

#include "CombatSystem.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"

namespace sage
{


void CombatSystem::onEnemyDead(entt::entity entity)
{
    auto& animation = registry->get<Animation>(entity);
    animation.ChangeAnimation(0, true); // TODO: Magic number for changing animations
}

void CombatSystem::startCombat(entt::entity entity)
{
    auto& player = registry->get<Transform>(actorMovementSystem->GetControlledActor());
    {
        entt::sink sink { player.onFinishMovement };
        sink.disconnect<&CombatSystem::startCombat>(this);
    }
    
    auto& animation = registry->get<Animation>(targetEnemy);
    animation.ChangeAnimation(1); // TODO: Magic number for changing animations
    auto& healthbar = registry->get<HealthBar>(targetEnemy);
    healthbar.Decrement(targetEnemy, 10); // TODO: tmp

    if (healthbar.hp <= 0)
    {
        onEnemyDead(targetEnemy);
    }
}

void CombatSystem::onEnemyClick(entt::entity entity)
{
    targetEnemy = entity;
    auto& player = registry->get<Transform>(actorMovementSystem->GetControlledActor());
    const auto& enemy = registry->get<Transform>(entity);
    actorMovementSystem->PathfindToLocation(actorMovementSystem->GetControlledActor(), enemy.position);
    {
        entt::sink sink { player.onFinishMovement };
        sink.connect<&CombatSystem::startCombat>(this);
    }
}


CombatSystem::CombatSystem(entt::registry *_registry, Cursor *_cursor, ActorMovementSystem* _actorMovementSystem) :
registry(_registry), cursor(_cursor), actorMovementSystem(_actorMovementSystem)
{
    {
        entt::sink sink{cursor->onEnemyClick};
        sink.connect<&CombatSystem::onEnemyClick>(this);  
    }
}
} // sage