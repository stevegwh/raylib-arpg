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
		AttackData attackData;
		TimerManager* timerManager;
        entt::registry* registry;
        CollisionSystem* collisionSystem;
        std::vector<entt::entity> hitUnits;
		bool active = false;
        float m_cooldownLimit;
		float m_cooldownTimer;
		float windupLimit;
		float windupTimer;
		float duration;
	public:
		virtual bool IsActive() const
		{
			return active;
		}
		float cooldownTimer() const
		{
			return m_cooldownTimer;
		}
		float cooldownLimit() const
		{
			return m_cooldownLimit;
		}
		virtual void Execute(entt::entity self) = 0;
        virtual void Update(entt::entity self);
		virtual void Draw3D(entt::entity self);
        virtual ~Ability() = default;
        Ability(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager);
	};
}