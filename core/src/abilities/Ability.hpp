#pragma once

#include <entt/entt.hpp>

#include <vector>

#include "components/CombatableActor.hpp"
#include "systems/CollisionSystem.hpp"
#include <Timer.hpp>

namespace sage
{
	class Ability
	{
	protected:
		Timer timer{};
		AttackData attackData;
        entt::registry* registry;
        CollisionSystem* collisionSystem;
        std::vector<entt::entity> hitUnits;
		bool active = false;
        float m_cooldownLimit;
		float m_windupLimit;
		float duration;
	public:
		virtual void ResetCooldown()
		{
			timer.Reset();
		}
		virtual bool IsActive() const
		{
			return active;
		}
		float cooldownTimer() const
		{
			return timer.RemainingTime();

		}
		float cooldownLimit() const
		{
			return timer.maxTime;
		}
		virtual void Execute(entt::entity self) = 0;
        virtual void Update(entt::entity self);
		virtual void Draw3D(entt::entity self);
		virtual void Init(entt::entity self);
        virtual ~Ability() = default;
		Ability(const Ability&) = delete;
		Ability& operator=(const Ability&) = delete;
        Ability(entt::registry* _registry, CollisionSystem* _collisionSystem);
	};
}