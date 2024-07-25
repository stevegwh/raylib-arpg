#pragma once

#include "Ability.hpp"
#include "TextureTerrainOverlay.hpp"
#include "Cursor.hpp"

namespace sage
{
	struct RainOfFireAbility : public Ability
	{
		Cursor* cursor;
		std::unique_ptr<TextureTerrainOverlay> spellCursor;
		float whirlwindRadius = 50.0f;
		void Use(entt::entity actor) override;
		void Update(entt::entity actor) override;
		void Confirm(entt::entity actor);
		~RainOfFireAbility() override = default;
		RainOfFireAbility(
				entt::registry* _registry,
				Cursor* _cursor,
				CollisionSystem* _collisionSystem,
				NavigationGridSystem* _navigationGridSystem,
				TimerManager* _timerManager);
	};
}