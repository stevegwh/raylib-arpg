#pragma once

#include "components/Animation.hpp"
#include <cereal/cereal.hpp>
#include <string>
#include <type_traits>

namespace sage
{
    enum class AbilityFunctionEnum;
    class Cursor;
    class VisualFX;

    enum class AbilityBehaviour : unsigned int
    {
        SPAWN_AT_CASTER = 1 << 0, // Ability starts at caster position
        SPAWN_AT_CURSOR = 1 << 1, // Ability starts at cursor position
        SPAWN_AT_TARGET = 1 << 2, // Ability starts at targeted unit's position

        FOLLOW_NONE = 1 << 3,   // Ability detaches from caster
        FOLLOW_CASTER = 1 << 4, // Ability follows caster's position (caster transform becomes parent)
        FOLLOW_TARGET = 1 << 5, // Ability follows target's position (target's transform becomes parent)

        MOVEMENT_STATIONARY = 1 << 6, // Ability transform stays still
        MOVEMENT_PROJECTILE = 1 << 7, // Ability transform moves over time
        MOVEMENT_HITSCAN = 1 << 8,    // Ability transform moves instantly (ability transform not needed)

        CAST_INSTANT = 1 << 9,    // No cast time
        CAST_CHANNELED = 1 << 10, // Periodic cast time
        CAST_REGULAR = 1 << 11,   // Cast time

        ATTACK_TARGET = 1 << 12,    // Attacks target
        ATTACK_POINT = 1 << 13,     // Attacks target's position at time of casting
        ATTACK_AOE_POINT = 1 << 14, // Attacks point in a radius
        ATTACK_AOE_TARGET = 1 << 15 // Attacks single target with a radius that can hit surrounding
    };

    enum class AbilityBehaviourOptional : unsigned int
    {
        INDICATOR = 1 << 0,
        REPEAT_AUTO = 1 << 1,
        INTERRUPTABLE = 1 << 2,
        CHANNELED_MOVABLE = 1 << 3,
        PIERCING = 1 << 4,
        DOT = 1 << 5
    };

    enum class AbilityElement : unsigned int
    {
        PHYSICAL = 1 << 0,
        EARTH = 1 << 1,
        WIND = 1 << 2,
        FIRE = 1 << 3,
        WATER = 1 << 4,
        LIGHTNING = 1 << 5,
        DARK = 1 << 6,
        LIGHT = 1 << 7
    };

    // Enable bitwise operations
    template <typename E>
    struct EnableBitMaskOperators
    {
        static const bool enable = false;
    };

    template <>
    struct EnableBitMaskOperators<AbilityBehaviour>
    {
        static const bool enable = true;
    };

    template <>
    struct EnableBitMaskOperators<AbilityBehaviourOptional>
    {
        static const bool enable = true;
    };

    template <>
    struct EnableBitMaskOperators<AbilityElement>
    {
        static const bool enable = true;
    };

    // Bitwise operators
    template <typename E>
    typename std::enable_if<EnableBitMaskOperators<E>::enable, E>::type operator|(E lhs, E rhs)
    {
        using underlying = typename std::underlying_type<E>::type;
        return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
    }

    template <typename E>
    typename std::enable_if<EnableBitMaskOperators<E>::enable, E>::type operator&(E lhs, E rhs)
    {
        using underlying = typename std::underlying_type<E>::type;
        return static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
    }

    template <typename E>
    typename std::enable_if<EnableBitMaskOperators<E>::enable, E&>::type operator|=(E& lhs, E rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    enum class AbilityTargetType
    {
        TARGET_NONE,
        TARGET_SELF,
        TARGET_FRIENDLY,
        TARGET_ENEMY,
        TARGET_ALL
    };

    enum class AbilityResource
    {
        NONE,
        MANA,
        HEALTH,
        ENERGY
    };

    enum class AbilityCastFail
    {
        OUT_OF_RANGE,
        INTERRUPTED
    };

    /**
     * Auto attack: REPEAT_AUTO | SPAWN_AT_CASTER (needed?) | CASTER_FOLLOW | MOVEMENT_HITSCAN | CAST_INSTANT |
     * ATTACK_TARGET (And TARGET_ENEMY)
     * Rain of fire: INDICATOR | SPAWN_AT_CURSOR | CASTER_DETACH | MOVEMENT_STATIONARY | CAST_CHANNELED |
     * ATTACK_AOE
     * Fireball : SPAWN_AT_CASTER | CASTER_DETACH | MOVEMENT_PROJECTILE | CAST_INSTANT | ATTACK_POINT
     */
    class AbilityData
    {
      public:
        std::string name;

        struct BaseData
        {
            float cooldownDuration = 0; // How long per tick or per use
            int baseDamage = 0;         // The base damage of the attack
            float range = 20;           // The range the ability can be cast
            float radius = 10;          // The radius of the ability from the attack point
            float castTime = 0;         // How long an ability takes to execute
            AbilityElement elements = static_cast<AbilityElement>(0); // The elements of the attack
            AbilityBehaviour behaviour = static_cast<AbilityBehaviour>(0);
            AbilityBehaviourOptional optional = static_cast<AbilityBehaviourOptional>(0);
            AbilityTargetType targetType = AbilityTargetType::TARGET_NONE;

            // Helper functions for behaviour flags
            void AddBehaviour(AbilityBehaviour b)
            {
                behaviour |= b;
            }
            void RemoveBehaviour(AbilityBehaviour b)
            {
                behaviour = static_cast<AbilityBehaviour>(
                    static_cast<unsigned int>(behaviour) & ~static_cast<unsigned int>(b));
            }
            bool HasBehaviour(AbilityBehaviour b) const
            {
                return (static_cast<unsigned int>(behaviour) & static_cast<unsigned int>(b)) != 0;
            }
            bool HasAllBehaviours(AbilityBehaviour b) const
            {
                return (static_cast<unsigned int>(behaviour) & static_cast<unsigned int>(b)) ==
                       static_cast<unsigned int>(b);
            }

            // Helper functions for optional behaviour flags
            void AddOptionalBehaviour(AbilityBehaviourOptional b)
            {
                optional |= b;
            }
            void RemoveOptionalBehaviour(AbilityBehaviourOptional b)
            {
                optional = static_cast<AbilityBehaviourOptional>(
                    static_cast<unsigned int>(optional) & ~static_cast<unsigned int>(b));
            }
            bool HasOptionalBehaviour(AbilityBehaviourOptional b) const
            {
                return (static_cast<unsigned int>(optional) & static_cast<unsigned int>(b)) != 0;
            }
            bool HasAllOptionalBehaviours(AbilityBehaviourOptional b) const
            {
                return (static_cast<unsigned int>(optional) & static_cast<unsigned int>(b)) ==
                       static_cast<unsigned int>(b);
            }

            // Helper functions for element flags
            void AddElement(AbilityElement e)
            {
                elements |= e;
            }
            void RemoveElement(AbilityElement e)
            {
                elements = static_cast<AbilityElement>(
                    static_cast<unsigned int>(elements) & ~static_cast<unsigned int>(e));
            }
            bool HasElement(AbilityElement e) const
            {
                return (static_cast<unsigned int>(elements) & static_cast<unsigned int>(e)) != 0;
            }
            bool HasAllElements(AbilityElement e) const
            {
                return (static_cast<unsigned int>(elements) & static_cast<unsigned int>(e)) ==
                       static_cast<unsigned int>(e);
            }
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
    };

} // namespace sage