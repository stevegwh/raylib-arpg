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
		float initialDamage;
		float damageOverTime; // 0 if no damage over time
		// A DoT should probably push a function to the affected unit and the ability's "DoT update" should be called every frame
		virtual void Use(entt::entity actor) = 0;
        virtual void Update(entt::entity actor);
		virtual void Draw3D(entt::entity actor);
        virtual ~Ability() = default;
        Ability(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager);
	};
}