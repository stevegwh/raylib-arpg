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
		if (!spellCursor->active())
		{
			spellCursor->Init(cursor->collision.point); // Should not do this if already done
			spellCursor->Enable(true);
			controllableActorSystem->Disable();
			return;
		}
		else
		{
			controllableActorSystem->Enable();
			spellCursor->Enable(false); // Cancel move
		}
	}
	
	void RainOfFireAbility::Confirm(entt::entity actor)
	{
		std::cout << "Rain of fire ability used \n";
		cooldownTimer = cooldownLimit;
		active = true;
		windupTimer = 0.0f;
		auto& animation = registry->get<Animation>(actor);
		animation.ChangeAnimationByEnum(AnimationEnum::SPIN, true);
		spellCursor->Enable(false);
		controllableActorSystem->Enable();
	}

	void RainOfFireAbility::Update(entt::entity actor)
	{
		if (spellCursor->active())
		{
			spellCursor->Update(cursor->collision.point);
			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !active)
			{
				Confirm(actor);
			}
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
			const auto& targetCol = registry->get<Collideable>(entity);

			if (CheckCollisionBoxSphere(targetCol.worldBoundingBox, cursor->collision.point, whirlwindRadius))
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
			ControllableActorSystem* _controllableActorSystem,
			TimerManager* _timerManager)
			:
			Ability(_registry, _collisionSystem, _timerManager),
			cursor(_cursor),
			controllableActorSystem(_controllableActorSystem)
	{
		windupTimer = 0.0f;
		windupLimit = 0.75f;
		cooldownLimit = 3.0f;
		initialDamage = 25.0f;
		spellCursor = std::make_unique<TextureTerrainOverlay>(registry, _navigationGridSystem,
				"resources/textures/cursor/rainoffire_cursor.png");
	}
}
