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
        Timer cooldownTimer{};
        AttackData attackData;
        entt::registry* registry;
        CollisionSystem* collisionSystem;
        bool active = false;
        float m_windupLimit;
        float duration;

      public:
        virtual void ResetCooldown()
        {
            cooldownTimer.Reset();
        }
        virtual bool IsActive() const
        {
            return active;
        }
        float GetRemainingCooldownTime() const
        {
            return cooldownTimer.GetRemainingTime();
        }
        float GetCooldownDuration() const
        {
            return cooldownTimer.GetMaxTime();
        }
        bool CooldownReady() const
        {
            return cooldownTimer.GetRemainingTime() <= 0;
        }
        virtual void Execute(entt::entity self) = 0;
        virtual void Update(entt::entity self);
        virtual void Draw3D(entt::entity self);
        virtual void Init(entt::entity self);
        virtual ~Ability() = default;
        Ability(const Ability&) = delete;
        Ability& operator=(const Ability&) = delete;
        Ability(
            entt::registry* _registry,
            float _cooldownDuration,
            CollisionSystem* _collisionSystem);
    };
} // namespace sage