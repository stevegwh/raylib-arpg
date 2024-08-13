#pragma once

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include <cereal/cereal.hpp>

#include <cereal/archives/json.hpp>

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
            float cooldownDuration = 0; // How long per tick or per use
            int baseDamage = 0;         // The base damage of the attack
            float range = 0;            // The range the ability can be cast
            float radius = 0;           // The radius of the ability from the attack point
            AttackElement element = AttackElement::PHYSICAL; // The element of the attack
            bool repeatable =
                false; // Whether the attack should automatically repeat when off cooldown
        };
        struct VisualFXData
        {
            std::string name = "";
            VisualFX* ptr = nullptr;
        };
        BaseData base{};
        AnimationParams animationParams{};
        VisualFXData vfx{};
        AbilityFunction* executeFunc;
        Cursor* cursor;

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(base, animationParams, vfx);
        }
    };

    template <typename Archive>
    void serialize(Archive& archive, AbilityData::BaseData& bd)
    {
        archive(
            CEREAL_NVP(bd.cooldownDuration),
            CEREAL_NVP(bd.baseDamage),
            CEREAL_NVP(bd.range),
            CEREAL_NVP(bd.radius),
            CEREAL_NVP(bd.element),
            CEREAL_NVP(bd.repeatable));
    };

    template <typename Archive>
    void serialize(Archive& archive, AbilityData::VisualFXData& vfx)
    {
        archive(CEREAL_NVP(vfx.name));
    };

    template <typename Archive>
    void serialize(Archive& archive, AnimationParams& anim)
    {
        archive(
            CEREAL_NVP(anim.animEnum),
            CEREAL_NVP(anim.animSpeed),
            CEREAL_NVP(anim.oneShot),
            CEREAL_NVP(anim.animationDelay));
    };

} // namespace sage