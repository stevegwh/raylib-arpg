#pragma once

#include "abilities/AbilityData.hpp"
#include <Timer.hpp>

#include <entt/entt.hpp>
#include <unordered_map>

namespace sage
{

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class AbilityState;
    class AbilityIndicator;
    class VisualFX;

    struct Ability
    {
        entt::entity self;
        entt::entity caster;
        AbilityData ad;
        Timer cooldownTimer;
        Timer executionDelayTimer;

        std::unique_ptr<VisualFX> vfx;
        std::unique_ptr<AbilityIndicator> abilityIndicator;

        AbilityStateEnum state = AbilityStateEnum::IDLE;

        void ResetCooldown();
        bool IsActive();
        float GetRemainingCooldownTime() const;
        float GetCooldownDuration() const;
        bool CooldownReady() const;
    };
} // namespace sage
