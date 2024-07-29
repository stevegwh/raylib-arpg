#pragma once

#include "Ability.hpp"
#include "TextureTerrainOverlay.hpp"
#include "Cursor.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "../utils/particle/RainOfFireVFX.hpp"

#include <memory>

namespace sage
{
	struct RainOfFireAbility : public Ability
	{
		std::unique_ptr<RainOfFireVFX> vfx;
		Cursor* cursor;
		ControllableActorSystem* controllableActorSystem;
		std::unique_ptr<TextureTerrainOverlay> spellCursor;
		float whirlwindRadius = 50.0f;
		void Use(entt::entity actor) override;
		void Draw3D(entt::entity actor) override;
		void Update(entt::entity actor) override;
		void Confirm(entt::entity actor);
		~RainOfFireAbility() override = default;
		RainOfFireAbility(
				entt::registry* _registry,
				Camera* _camera,
				Cursor* _cursor,
				CollisionSystem* _collisionSystem,
				NavigationGridSystem* _navigationGridSystem,
				ControllableActorSystem* _controllableActorSystem,
				TimerManager* _timerManager);
	};
}