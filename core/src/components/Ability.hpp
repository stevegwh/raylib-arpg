#pragma once

#include "abilities/AbilityData.hpp"
#include "abilities/AbilityIndicator.hpp"
#include "abilities/vfx/VisualFX.hpp"
#include <Timer.hpp>

#include <entt/entt.hpp>
#include <unordered_map>

namespace sage
{
    struct Ability
    {
        entt::entity self{};
        entt::entity caster{};
        AbilityData ad;
        Timer cooldownTimer;
        Timer castTimer;

        std::string name;
        std::string description;
        AssetID icon{};
        std::string iconPath; // Use AssetID where possible

        std::unique_ptr<VisualFX> vfx;
        // TODO: VFX should have before, during and after.

        std::unique_ptr<AbilityIndicator> abilityIndicator;

        void ResetCooldown();
        bool IsActive();
        [[nodiscard]] float GetRemainingCooldownTime() const;
        [[nodiscard]] float GetCooldownDuration() const;
        [[nodiscard]] bool CooldownReady() const;

        entt::sigh<void(entt::entity)> startCast;
        entt::sigh<void(entt::entity)> cancelCast;
        entt::sigh<void(entt::entity, AbilityCastFail)> castFailed;

        Ability() = default;
        Ability(const Ability&) = delete;
        Ability& operator=(const Ability&) = delete;
    };
} // namespace sage
