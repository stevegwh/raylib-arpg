#pragma once

#include "Ability.hpp"

namespace sage
{
	struct ConeOfCold : public Ability
	{
		float whirlwindRadius = 50.0f;
		void Execute(entt::entity actor) override;
		void Update(entt::entity actor) override;
		~ConeOfCold() override = default;
		ConeOfCold(entt::registry* _registry, CollisionSystem* _collisionSystem);
	};
}