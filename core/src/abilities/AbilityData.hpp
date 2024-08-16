#pragma once

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include <cereal/cereal.hpp>

namespace sage
{

    class AbilityFunction;
    class Cursor;
    class VisualFX;

    // Make a serialization function that deserialized this struct and returns the correct
    // ability for it.
    class AbilityData
    {
      public:
        struct BaseData
        {
            float cooldownDuration = 0;                      // How long per tick or per use
            int baseDamage = 0;                              // The base damage of the attack
            float range = 0;                                 // The range the ability can be cast
            float radius = 0;                                // The radius of the ability from the attack point
            AttackElement element = AttackElement::PHYSICAL; // The element of the attack
            bool repeatable = false;          // Whether the attack should automatically repeat when off cooldown
            std::string executeFuncName = ""; // Name of function to call on execute
        };
        struct VisualFXData
        {
            std::string name = "";
            VisualFX* ptr = nullptr;

            // VFX behaviour? (Follow player, cast to cursor, )
        };
        struct IndicatorData
        {
            std::string indicatorKey = ""; // Key to an indicator/cursor class that has the texture etc.
            Cursor* cursor;
            // indicator behaviour? (spawn at cursor, spawn at player, spawn at target)
        };
        BaseData base{};
        AnimationParams animationParams{};
        VisualFXData vfx{};
        IndicatorData indicator{};

        Cursor* cursor;
        AbilityFunction* executeFunc;
    };

} // namespace sage