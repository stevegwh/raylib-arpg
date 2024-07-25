//
// Created by Steve Wheeler on 21/07/2024.
//

#include "RainOfFireAbility.hpp"
#include "raylib.h"
#include "components/sgTransform.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"

namespace sage
{
	void RainOfFireAbility::Use(entt::entity actor)
	{
		if (cooldownTimer > 0)
		{
			std::cout << "Waiting for cooldown \n";
			return;
		}
		std::cout << "Rain of fire ability used \n";
		if (!cursorActive)
		{
			cursorActive = true;
			spellCursor->Init(cursor->collision.point);
			return;
		}
		cooldownTimer = cooldownLimit;
		active = true;
		windupTimer = 0.0f;
//		auto& animation = registry->get<Animation>(actor);
//		animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
	}

	void RainOfFireAbility::Update(entt::entity actor)
	{
		if (cursorActive)
		{
			spellCursor->Update(cursor->collision.point);
			return;
		}
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
				combatable.onHit.publish(entity, actor, initialDamage);
				std::cout << "Hit unit \n";
			}
		}
		active = false;
	}

	RainOfFireAbility::RainOfFireAbility(
			entt::registry* _registry,
			Cursor* _cursor,
			CollisionSystem* _collisionSystem,
			NavigationGridSystem* _navigationGridSystem,
			TimerManager* _timerManager)
			:
			Ability(_registry, _collisionSystem, _timerManager),
			cursor(_cursor)
	{
		windupTimer = 0.0f;
		windupLimit = 0.75f;
		cooldownLimit = 3.0f;
		initialDamage = 25.0f;
		spellCursor = std::make_unique<TextureTerrainOverlay>(registry, _navigationGridSystem,
				"resources/textures/cursor/rainoffire_cursor.png");
	}
}
