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
    void WhirlwindAbility::Use(entt::entity actor)
    {
        if (cooldownTimer > 0)
        {
			std::cout << "Waiting for cooldown \n";
            return;
        }

		std::cout << "Whirlwind ability used \n";
		cooldownTimer = cooldownLimit;
		auto& actorTransform = registry->get<sgTransform>(actor);
		const auto& actorCol = registry->get<Collideable>(actor);
		auto& animation = registry->get<Animation>(actor);
		animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
		
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
				combatable.onHit.publish(entity, actor, initialDamage);
				std::cout << "Hit unit \n";
			}
		}
    }
    
    void WhirlwindAbility::Update(entt::entity actor)
    {
        cooldownTimer -= GetFrameTime();
    }
    
    WhirlwindAbility::WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem) : 
    Ability(_registry, _collisionSystem)
    {
        cooldownLimit = 3.0f;
        initialDamage = 10.0f;
    }
}
