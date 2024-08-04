#pragma once

#include "Ability.hpp"

namespace sage
{
	struct WhirlwindAbility : public Ability
	{
		float whirlwindRadius = 15.0f;
        void Execute(entt::entity actor) override;
        void Update(entt::entity actor) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager);
	};
}