//
// Created by steve on 05/06/24.
//

#include "raylib.h"
#include "raymath.h"

#include "WaveMobCombatLogicSubSystem.hpp"
#include "../components/CombatableActor.hpp"
#include "../components/Animation.hpp"
#include "../components/Transform.hpp"
#include "../components/HealthBar.hpp"

namespace sage
{
void WaveMobCombatLogicSubSystem::Update(entt::entity entity)
{
	CheckInCombat(entity);
	auto& c = registry->get<CombatableActor>(entity);
	if (c.target == entt::null || !c.inCombat) return;
	// Move to target
	// Progress tick
	// Turn to look at target
	// Autoattack

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

void WaveMobCombatLogicSubSystem::CheckInCombat(entt::entity entity) const
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
    {
        entt::sink sink { combatable.onHit };
        sink.disconnect<&WaveMobCombatLogicSubSystem::OnHit>(this);
    }
    {
        entt::sink sink { combatable.onDeath };
        sink.disconnect<&WaveMobCombatLogicSubSystem::OnDeath>(this);
    }
    
    combatable.inCombat = false;
    auto& animation = registry->get<Animation>(entity);
    animation.ChangeAnimationByEnum(AnimationEnum::DEATH, true);
    {
        entt::sink sink { animation.onAnimationEnd };
        sink.connect<&WaveMobCombatLogicSubSystem::destroyEnemy>(this);
    }
    auto& c = registry->get<CombatableActor>(entity);
    c.target = entt::null;
}

void WaveMobCombatLogicSubSystem::AutoAttack(entt::entity entity) const
{
	auto& c = registry->get<CombatableActor>(entity);
	auto& t = registry->get<Transform>(entity);
	auto& enemyPos = registry->get<Transform>(c.target).position;
	Vector3 direction = Vector3Subtract(enemyPos, t.position);
	float angle = atan2f(direction.x, direction.z) * RAD2DEG;
	t.rotation.y = angle;
	c.autoAttackTick = 0;

	auto& animation = registry->get<Animation>(entity);
	animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK);
}

void WaveMobCombatLogicSubSystem::StartCombat(entt::entity entity)
{

}

void WaveMobCombatLogicSubSystem::OnHit(entt::entity entity, entt::entity attacker) const
{
    
    // Aggro when player hits
    auto& c = registry->get<CombatableActor>(entity);
    c.target = attacker;
    c.inCombat = true;
}

WaveMobCombatLogicSubSystem::WaveMobCombatLogicSubSystem(entt::registry *_registry) :
	registry(_registry)
{
    // TODO: This method does not work because the GameObjectFactory is run after this constructor
    auto view = registry->view<CombatableActor>();
    for (auto& entity: view) 
    {
        auto& combatable = registry->get<CombatableActor>(entity);
        if (combatable.actorType != CombatableActorType::WAVEMOB) 
        {
            continue;
        }
        
        {
            entt::sink sink { combatable.onHit };
            sink.connect<&WaveMobCombatLogicSubSystem::OnHit>(this);
        }
        {
            entt::sink sink { combatable.onDeath };
            sink.connect<&WaveMobCombatLogicSubSystem::OnDeath>(this);
        }
    }

}
} // sage