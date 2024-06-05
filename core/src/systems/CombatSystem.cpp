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
    auto& combatable = registry->get<CombatableActor>(entity);
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
    auto& c = registry->get<CombatableActor>(entity);
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
    auto& playerCombatable = registry->get<CombatableActor>(actorMovementSystem->GetControlledActor());
    playerCombatable.inCombat = true;
    
    // TODO: This should not be the way of starting combat with an enemy (should be on hit)
    auto& enemyCombatable = registry->get<CombatableActor>(playerCombatable.target);
    enemyCombatable.target = actorMovementSystem->GetControlledActor();
    enemyCombatable.inCombat = true;
}

void CombatSystem::onAutoAttackEnd(entt::entity entity)
{
}

void CombatSystem::checkInCombat(entt::entity entity)
{
    // If the entity is not the target of any other combatable.
	// If no current target
	// Have a timer for aggro and if the player is not within that range for a certain amount of time they resume their regular task (tasks TBC)
	auto& combatable = registry->get<CombatableActor>(entity);
	if (combatable.target == entt::null)
	{
		combatable.inCombat = false;
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
	}
}

void CombatSystem::onEnemyClick(entt::entity entity)
{
    auto& combatable = registry->get<CombatableActor>(actorMovementSystem->GetControlledActor());
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
    auto view = registry->view<CombatableActor, Transform>();
    for (auto& entity : view) 
    {
        auto& c = registry->get<CombatableActor>(entity);
		if (c.actorType == CombatableActorType::PLAYER)
		{
			playerCombatLogicSubSystem->Update(entity);
		}
		else if (c.actorType == CombatableActorType::WAVEMOB)
		{
			waveMobCombatLogicSubSystem->Update(entity);
		}
    }
}

CombatSystem::CombatSystem(entt::registry *_registry, Cursor *_cursor, ControllableActorMovementSystem* _actorMovementSystem) :
	BaseSystem<CombatableActor>(_registry), cursor(_cursor), actorMovementSystem(_actorMovementSystem)
{
    {
        entt::sink sink{ cursor->onEnemyClick };
        sink.connect<&CombatSystem::onEnemyClick>(this);  
    }
}
} // sage