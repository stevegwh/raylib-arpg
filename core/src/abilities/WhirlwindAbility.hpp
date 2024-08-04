#pragma once

#include "Ability.hpp"

namespace sage
{
	struct WhirlwindAbility : public Ability
	{
		float whirlwindRadius = 15.0f;
        void Init(entt::entity self) override;
        void Execute(entt::entity self) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager);
	};
}