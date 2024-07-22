#pragma once

#include "Ability.hpp"

namespace sage
{
	struct WhirlwindAbility : public Ability
	{
		float whirlwindRadius = 50.0f;
        void Use(entt::entity actor) override;
        void Update(entt::entity actor) override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem);
	};
}