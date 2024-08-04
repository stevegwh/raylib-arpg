#pragma once

#include "Ability.hpp"

namespace sage
{
	struct PlayerAutoAttack : public Ability
	{
        entt::delegate<void()> timerCallback;
        void Execute(entt::entity self) override;
        void Update(entt::entity self) override;
        void Cancel();
        ~PlayerAutoAttack() override = default;
        PlayerAutoAttack(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager);
	};
}