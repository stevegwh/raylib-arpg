#pragma once

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

namespace sage
{
    class AbilityFunction;
    // Make a serialization function that deserialized this struct and returns the correct
    // ability for it.
    struct AbilityData
    {
        struct BaseData
        {
            float cooldownDuration;
            float range;
            int baseDamage;
            AttackElement element = AttackElement::PHYSICAL;
            bool repeatable = false;
        };
        BaseData baseData;
        AnimationParams animationParams;
        AbilityFunction* executeFunc;
    }; // namespace sage

} // namespace sage