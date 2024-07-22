#pragma once

#include "Ability.hpp"

namespace sage
{
	struct WhirlwindAbility : public Ability
	{
        void Use() override;
        void Update() override;
        ~WhirlwindAbility() override = default;
        WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem);
	};
}