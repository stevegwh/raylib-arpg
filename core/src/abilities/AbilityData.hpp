#pragma once

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

namespace sage
{
    class AbilityFunction;
    class Cursor;
    class VisualFX;
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
        struct VisualFXData
        {
            std::string name;
            VisualFX* ptr;
        };
        BaseData baseData;
        AnimationParams animationParams;
        VisualFXData vfx;
        AbilityFunction* executeFunc;
        Cursor* cursor;
    }; // namespace sage

} // namespace sage