#pragma once

#include <Timer.hpp>

#include <entt/entt.hpp>
#include <unordered_map>

namespace sage
{
    class AbilityState;
    class AbilityStateEnum;
    class VisualFX;

    struct Ability
    {
        entt::entity self;
        entt::entity caster;
        Timer cooldownTimer;
        Timer executionDelayTimer;

        std::unique_ptr<VisualFX> vfx;

        AbilityState* state;
    };
} // namespace sage
