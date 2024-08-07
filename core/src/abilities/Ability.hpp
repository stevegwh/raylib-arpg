#pragma once

#include <entt/entt.hpp>

#include <vector>

#include "components/CombatableActor.hpp"
#include "systems/CollisionSystem.hpp"
#include <Timer.hpp>

namespace sage
{
    struct AbilityData
    {
        float cooldownDuration;
        float range;
        int baseDamage;
        AttackElement element;
    };
    
    class Ability
    {
      protected:
        Timer cooldownTimer{};
        AttackData attackData;
        entt::registry* registry;
        CollisionSystem* collisionSystem;
        bool active = false;

      public:
        virtual void ResetCooldown();
        virtual bool IsActive() const;
        float GetRemainingCooldownTime() const;
        float GetCooldownDuration() const;
        bool CooldownReady() const;
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