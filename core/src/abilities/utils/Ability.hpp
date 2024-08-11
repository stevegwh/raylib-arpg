#pragma once

#include "components/CombatableActor.hpp"
#include "particle/RainOfFireVFX.hpp"
#include "TextureTerrainOverlay.hpp"
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
        AttackElement element = AttackElement::PHYSICAL;
        float animationDelay = 0;
        bool repeatable = false;
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
        entt::registry* registry;
        Timer cooldownTimer{};
        Timer animationDelayTimer{};
        std::unique_ptr<RainOfFireVFX> vfx; // TODO: make a generic VFX class
        std::unique_ptr<TextureTerrainOverlay> spellCursor;
        AbilityData abilityData;
        std::unordered_map<AbilityState, std::unique_ptr<State>> states;
        State* state;
        void ChangeState(entt::entity self, AbilityState newState);

        class IdleState : public State
        {
            Ability* ability;

          public:
            void Update(entt::entity self) override;
            void Draw3D(entt::entity self) override;
            IdleState(Ability* _ability) : ability(_ability)
            {
            }
        };

        class AwaitingExecutionState : public State
        {
            Ability* ability;

          public:
            void OnEnter(entt::entity self) override;
            void Update(entt::entity self) override;
            AwaitingExecutionState(Ability* _ability) : ability(_ability)
            {
            }
        };

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