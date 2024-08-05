#pragma once

#include <entt/entt.hpp>

#include <vector>

#include "components/CombatableActor.hpp"
#include "systems/CollisionSystem.hpp"
#include "TimerManager.hpp"

namespace sage
{
	class Ability
	{
	protected:
		int cooldownTimerId = -1;
		int windupTimerId = -1;
		AttackData attackData;
		TimerManager* timerManager;
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
			timerManager->RemoveTimer(cooldownTimerId);
			cooldownTimerId = -1;
		}
		virtual bool IsActive() const
		{
			return active;
		}
		float cooldownTimer() const
		{
			float time = timerManager->GetRemainingTime(cooldownTimerId);
			if (time == -1) time = 0;
			return time;
		}
		float cooldownLimit() const
		{
			return m_cooldownLimit;
		}
		virtual void Execute(entt::entity self) = 0;
        virtual void Update(entt::entity self);
		virtual void Draw3D(entt::entity self);
		virtual void Init(entt::entity self);
        virtual ~Ability() = default;
		Ability(const Ability&) = delete;
		Ability& operator=(const Ability&) = delete;
        Ability(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager);
	};
}