#pragma once

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include <cereal/cereal.hpp>
#include <string>

namespace sage
{

    class AbilityFunction;
    enum class AbilityFunctionEnum;
    class Cursor;
    class VisualFX;

    enum class AbilitySpawnBehaviour
    {
        AT_CASTER,
        AT_CURSOR
    };

    enum class AbilityBehaviourOnHit // Hit Behaviour? Basically, what can this ability hit
    {
        HIT_TARGETED_UNIT, // Hits a particular unit
        HIT_ALL_IN_RADIUS, // Hits all in a spherical radius
        PASSIVE            // Will not seek to hit any enemies
    };

    enum class AbilityBehaviourPreHit
    {
        FOLLOW_CASTER,       // Follows the caster's movement
        DETACHED_PROJECTILE, // Detaches from player and moves to a target
        DETACHED_STATIONARY  // Deteaches from player and remains still
    };

    // Make a serialization function that deserialized this struct and returns the correct
    // ability for it.
    class AbilityData
    {
      public:
        std::string name;
        struct BaseData
        {
            float cooldownDuration = 0;                      // How long per tick or per use
            int baseDamage = 0;                              // The base damage of the attack
            float range = 20;                                // The range the ability can be cast
            float radius = 10;                               // The radius of the ability from the attack point
            AttackElement element = AttackElement::PHYSICAL; // The element of the attack
            bool repeatable = false; // Whether the attack should automatically repeat when off cooldown
            AbilitySpawnBehaviour spawnBehaviour = AbilitySpawnBehaviour::AT_CASTER;
            AbilityBehaviourOnHit behaviourOnHit = AbilityBehaviourOnHit::HIT_TARGETED_UNIT;
            AbilityBehaviourPreHit behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;
        };
        struct VisualFXData
        {
            std::string name = "";
            VisualFX* ptr = nullptr;
        };
        struct IndicatorData
        {
            std::string indicatorKey = ""; // Key to an indicator/cursor class that has the texture etc.
        };
        BaseData base{};
        AnimationParams animationParams{};
        VisualFXData vfx{};
        IndicatorData indicator{};
        bool requiresIndicator = false;
    };

} // namespace sage