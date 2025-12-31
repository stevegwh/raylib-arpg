#pragma once

#include "abilities/AbilityData.hpp"
#include "abilities/AbilityIndicator.hpp"
#include "engine/Timer.hpp"

#include "entt/entt.hpp"

#include <unordered_map>

namespace lq
{
    class VisualFX;

    struct Ability
    {
        entt::entity self{};
        entt::entity caster{};
        Timer cooldownTimer;
        Timer castTimer;

        // TODO: Feels strange to have subscriptions and events in the same struct?
        sage::Subscription onStartCastSub{};
        sage::Subscription onCancelCastSub{};

        AssetID icon{};
        std::string iconPath; // Use AssetID where possible
        // TODO: VFX should have before, during and after.
        VisualFX* GetVfx(entt::registry* registry) const;
        std::unique_ptr<AbilityIndicator> abilityIndicator{};

        void ResetCooldown();
        bool IsActive();
        [[nodiscard]] float GetRemainingCooldownTime() const;
        [[nodiscard]] float GetCooldownDuration() const;
        [[nodiscard]] bool CooldownReady() const;

        sage::Event<entt::entity> startCast{};
        sage::Event<entt::entity> cancelCast{};
        sage::Event<entt::entity, AbilityCastFail> castFailed{};

        Ability() = default;
        Ability(const Ability&) = delete;
        Ability& operator=(const Ability&) = delete;
    };
} // namespace lq
