#pragma once

#include <entt/entt.hpp>

#include <vector>

#include "components/CombatableActor.hpp"
#include "systems/CollisionSystem.hpp"
#include "TimerManager.hpp"

namespace sage
{
	struct Ability
	{
		AttackData attackData;
		TimerManager* timerManager;
        entt::registry* registry;
        CollisionSystem* collisionSystem;
        std::vector<entt::entity> hitUnits;
		bool active = false;
        float cooldownLimit;
		float cooldownTimer;
		float windupLimit;
		float windupTimer;
		float duration;
		virtual void Execute(entt::entity self) = 0;
        virtual void Update(entt::entity self);
		virtual void Draw3D(entt::entity self);
        virtual ~Ability() = default;
        Ability(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager);
	};
}