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
    class AbilityState;

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class Ability
    {
        class IdleState;
        class AwaitingExecutionState;

      protected:
        entt::registry* registry;
        Timer cooldownTimer;
        Timer animationDelayTimer;
        std::unique_ptr<VisualFX> vfx;
        AbilityData abilityData;

        AbilityState* state;
        std::unordered_map<AbilityStateEnum, std::unique_ptr<AbilityState>> states;
        void ChangeState(entt::entity self, AbilityStateEnum newState);

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