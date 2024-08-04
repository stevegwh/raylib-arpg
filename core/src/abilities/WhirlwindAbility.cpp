//
// Created by Steve Wheeler on 21/07/2024.
//

#include "WhirlwindAbility.hpp"
#include "raylib.h"
#include "components/sgTransform.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"

namespace sage
{
    void WhirlwindAbility::Execute(entt::entity actor)
    {
        if (cooldownTimer > 0)
        {
			std::cout << "Waiting for cooldown \n";
            return;
        }

		std::cout << "Whirlwind ability used \n";
		cooldownTimer = cooldownLimit;
		active = true;
		windupTimer = 0.0f;
		auto& animation = registry->get<Animation>(actor);
		animation.ChangeAnimationByEnum(AnimationEnum::SPIN, 3, true);
    }
    
    void WhirlwindAbility::Update(entt::entity actor)
    {
        cooldownTimer -= GetFrameTime();
		if (windupTimer < windupLimit)
		{
			windupTimer += GetFrameTime();
			return;
		}
		if (!active) return;
        auto& actorTransform = registry->get<sgTransform>(actor);
		const auto& actorCol = registry->get<Collideable>(actor);
		
		auto view = registry->view<CombatableActor>();
		
		for (auto& entity : view)
		{
			if (entity == actor) continue;
			//if (std::find(hitUnits.begin(), hitUnits.end(), entity) != hitUnits.end()) continue;
			
			const auto& targetTransform = registry->get<sgTransform>(entity);
			const auto& targetCol = registry->get<Collideable>(entity);
			
			if (CheckCollisionBoxSphere(targetCol.worldBoundingBox, actorTransform.position(), whirlwindRadius))
			{
				//hitUnits.push_back(entity);
				const auto& combatable = registry->get<CombatableActor>(entity);
				AttackData _attackData = attackData;
				_attackData.hit = entity;
				_attackData.attacker = actor;
				combatable.onHit.publish(_attackData);
				std::cout << "Hit unit \n";
			}
		}
		active = false;
    }
    
    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager) : 
    Ability(_registry, _collisionSystem, _timerManager)
    {
		windupTimer = 0.0f;
		windupLimit = 0.65f;
        cooldownLimit = 3.0f;
		
		attackData.damage = 25.0f;
    }
}
