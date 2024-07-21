#pragma once

#include "Ability.hpp"

namespace sage
{
	struct WhirlwindAbility : public Ability
	{
        void Update() override;
        WhirlwindAbility(entt::registry* _registry, CollisionSystem* _collisionSystem);
	};
}