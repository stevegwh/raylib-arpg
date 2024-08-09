#pragma once

#include "components/CombatableActor.hpp"
#include <Timer.hpp>

#include <entt/entt.hpp>

#include <vector>

namespace sage
{

    struct AbilityData
    {
        float cooldownDuration;
        float range;
        int baseDamage;
        AttackElement element;
        float animationDelay;
    };

    enum class AbilityState
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    struct State
    {
        virtual ~State() = default;
        virtual void Update(entt::entity self) {};
        virtual void Draw3D(entt::entity self) {};
        virtual void OnEnter(entt::entity self) {};
        virtual void OnExit(entt::entity self) {};
    };

    class Ability
    {
      protected:
        Timer cooldownTimer{};
        AbilityData abilityData;
        entt::registry* registry;
        std::unordered_map<AbilityState, std::unique_ptr<State>> states;
        State* state;

        void ChangeState(entt::entity self, AbilityState newState);

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
        Ability(entt::registry* _registry, const AbilityData& _abilityData);
    };
} // namespace sage