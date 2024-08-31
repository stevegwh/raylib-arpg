#pragma once

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include <cereal/cereal.hpp>
#include <string>

// "ability_behavoir":
//	DOTA_ABILITY_BEHAVIOR_HIDDEN = 1 : This ability can be owned by a unit but can't be casted and wont show up on
// the HUD. 	DOTA_ABILITY_BEHAVIOR_PASSIVE = 2 : Can't be casted like above but this one shows up on the ability
// HUD 	DOTA_ABILITY_BEHAVIOR_NO_TARGET = 4 : Doesn't need a target to be cast, ability fires off as soon as the
// button is pressed 	DOTA_ABILITY_BEHAVIOR_UNIT_TARGET = 8 : Ability needs a target to be casted on.
//	DOTA_ABILITY_BEHAVIOR_POINT = 16 : Ability can be cast anywhere the mouse cursor is (If a unit is clicked it
// will just be cast where the unit was standing) 	DOTA_ABILITY_BEHAVIOR_AOE = 32 : This ability draws a radius
// where the ability will have effect. YOU STILL NEED A TARGETTING BEHAVIOR LIKE DOTA_ABILITY_BEHAVIOR_POINT FOR
// THIS TO WORK. 	DOTA_ABILITY_BEHAVIOR_NOT_LEARNABLE = 64 : This ability probably can be casted or have a
// casting scheme but cannot be learned (these are usually abilities that are temporary like techie's bomb
// detonate) 	DOTA_ABILITY_BEHAVIOR_CHANNELLED = 128 : This abillity is channelled. If the user moves or is
// silenced
// the ability is interrupted. 	DOTA_ABILITY_BEHAVIOR_ITEM = 256 : This ability is tied up to an item.
//	DOTA_ABILITY_BEHAVIOR_TOGGLE = 512 :
//
// "ability_unit_target_type":
//	DOTA_UNIT_TARGET_NONE = 0 :
//	DOTA_UNIT_TARGET_FRIENDLY_HERO = 5 :
//  DOTA_UNIT_TARGET_FRIENDLY_BASIC = 9 :
//	DOTA_UNIT_TARGET_FRIENDLY = 13 :
//	DOTA_UNIT_TARGET_ENEMY_HERO = 6 :
//	DOTA_UNIT_TARGET_ENEMY_BASIC = 10 :
//	DOTA_UNIT_TARGET_ENEMY = 14 :
//	DOTA_UNIT_TARGET_ALL = 15 :

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
        ATTACK_TARGET, // Hits a particular unit
        ATTACK_POINT,
        ATTACK_AOE,    // Hits all in a spherical radius
        ATTACK_PASSIVE // Will not seek to hit any enemies
    };

    enum class AbilityBehaviourPreHit
    {
        FOLLOW_CASTER,       // Follows the caster's movement
        DETACHED_PROJECTILE, // Detaches from player and moves to a target
        DETACHED_STATIONARY  // Deteaches from player and remains still
    };

    /*
    enum class AbilityBehaviour
    {
        SPAWN_AT_CASTER, // Ability starts at caster position
        SPAWN_AT_CURSOR, // Ability starts at cursor position
        SPAWN_AT_TARGET, // Ability starts at targeted unit's position

        FOLLOW_NONE, // Ability detaches from caster
        FOLLOW_CASTER, // Ability follows caster's position (caster transform becomes parent)
        FOLLOW_TARGET, // Ability follows target's position (target's transform becomes parent)
        // Could have FOLLOW_CASTER, maybe

        MOVEMENT_STATIONARY, // Ability transform stays still
        MOVEMENT_PROJECTILE, // Ability transform moves over time
        MOVEMENT_HITSCAN,  // Ability transform moves instantly (ability transform not needed)

        CAST_INSTANT, // No cast time
        CAST_CHANNELED, // Periodic cast time
        CAST_REGULAR, // Cast time

        ATTACK_TARGET, // Attacks target
        ATTACK_POINT, // Attacks target's position at time of casting
        ATTACK_AOE_POINT, // Attacks point in a radius
        ATTACK_AOE_TARGET // Attacks single target with a radius that can hit surrounding
    }

    enum class AbilityBehaviourOptional
    {
        INDICATOR,
        REPEAT_AUTO,
        INTERRUPTABLE,
        CHANNELED_MOVABLE,
        PIERCING,
        DOT
    }

    enum class AbilityTargetType
    {
        TARGET_NONE,
        TARGET_SELF,
        TARGET_FRIENDLY,
        TARGET_ENEMY,
        TARGET_ALL
    }

    enum class AbilityElement
    {
        PHYSICAL,
        EARTH,
        WIND,
        FIRE,
        WATER,
        LIGHTNING,
        DARK,
        LIGHT
    }

    enum clas AbilityResource
    {
        NONE,
        MANA,
        HEALTH,
        ENERGY
    };

    Auto attack: REPEAT_AUTO | SPAWN_AT_CASTER (needed?) | CASTER_FOLLOW | MOVEMENT_HITSCAN | CAST_INSTANT |
    ATTACK_TARGET (And TARGET_ENEMY)

    Rain of fire: INDICATOR | SPAWN_AT_CURSOR | CASTER_DETACH | MOVEMENT_STATIONARY | CAST_CHANNELED | ATTACK_AOE

    Fireball : SPAWN_AT_CASTER | CASTER_DETACH | MOVEMENT_PROJECTILE | CAST_INSTANT | ATTACK_POINT
    */

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
            AbilityBehaviourOnHit behaviourOnHit = AbilityBehaviourOnHit::ATTACK_TARGET;
            AbilityBehaviourPreHit behaviourPreHit = AbilityBehaviourPreHit::FOLLOW_CASTER;
        };
        struct VisualFXData
        {
            std::string name = "";
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