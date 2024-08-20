#pragma once

#include "AbilityData.hpp"
#include "Timer.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

namespace sage
{
    class GameData;
    class VisualFX;

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class Ability
    {
      protected:
        entt::registry* registry;
        Timer cooldownTimer;
        Timer animationDelayTimer;
        std::unique_ptr<VisualFX> vfx;
        AbilityData abilityData;

        class AbilityState
        {
          public:
            Timer& cooldownTimer;
            Timer& animationDelayTimer;

            virtual ~AbilityState() = default;
            virtual void Update(entt::entity self)
            {
            }
            virtual void Draw3D(entt::entity self)
            {
            }
            virtual void OnEnter(entt::entity self)
            {
            }
            virtual void OnExit(entt::entity self)
            {
            }

            AbilityState(Timer& cooldownTimer, Timer& animationDelayTimer)
                : cooldownTimer(cooldownTimer), animationDelayTimer(animationDelayTimer)
            {
            }
        };

        AbilityState* state;
        std::unordered_map<AbilityStateEnum, std::unique_ptr<AbilityState>> states;

        void ChangeState(entt::entity self, AbilityStateEnum newState);

        class IdleState;
        class AwaitingExecutionState;

      public:
        virtual void ResetCooldown();
        virtual bool IsActive() const;
        float GetRemainingCooldownTime() const;
        float GetCooldownDuration() const;
        bool CooldownReady() const;

        virtual void Cancel(entt::entity self);
        virtual void Execute(entt::entity self);
        virtual void Update(entt::entity self);
        virtual void Draw3D(entt::entity self);
        virtual void Init(entt::entity self);

        virtual ~Ability();
        Ability(const Ability&) = delete;
        Ability& operator=(const Ability&) = delete;
        Ability(entt::registry* registry, const AbilityData& abilityData, GameData* gameData);
    };

} // namespace sage