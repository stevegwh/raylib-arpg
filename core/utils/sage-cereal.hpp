//
// Created by Steve Wheeler on 01/12/2024.
//

#pragma once

#include "abilities/AbilityData.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"

namespace sage
{
    template <typename Archive>
    void save(Archive& archive, AbilityData::BaseData const& bd)
    {
        // TODO:
        archive(
            cereal::make_nvp("cooldownDuration", bd.cooldownDuration),
            cereal::make_nvp("baseDamage", bd.baseDamage),
            cereal::make_nvp("range", bd.range),
            cereal::make_nvp("radius", bd.radius)
            // cereal::make_nvp("element", bd.element),
            // cereal::make_nvp("repeatable", bd.repeatable)
            // cereal::make_nvp("executeFuncName", bd.executeFuncName)
        );
    }

    template <typename Archive>
    void load(Archive& archive, AbilityData::BaseData& bd)
    {
        // TODO:
        archive(
            cereal::make_nvp("cooldownDuration", bd.cooldownDuration),
            cereal::make_nvp("baseDamage", bd.baseDamage),
            cereal::make_nvp("range", bd.range),
            cereal::make_nvp("radius", bd.radius)
            // cereal::make_nvp("element", bd.element),
            // cereal::make_nvp("repeatable", bd.repeatable)
            // cereal::make_nvp("executeFuncName", bd.executeFuncName)
        );
    }

    template <typename Archive>
    void serialize(Archive& archive, AbilityData::VisualFXData& vfx)
    {
        archive(cereal::make_nvp("name", vfx.name));
    }

    template <typename Archive>
    void serialize(Archive& archive, AnimationParams& anim)
    {
        archive(
            cereal::make_nvp("animEnum", anim.animEnum),
            cereal::make_nvp("animSpeed", anim.animSpeed),
            cereal::make_nvp("oneShot", anim.oneShot),
            cereal::make_nvp("animationDelay", anim.animationDelay));
    }

    template <class Archive>
    void save(Archive& archive, const AbilityData& ad)
    {
        archive(
            cereal::make_nvp("base", ad.base),
            cereal::make_nvp("animationParams", ad.animationParams),
            cereal::make_nvp("vfx", ad.vfx));
    }

    template <class Archive>
    void load(Archive& archive, AbilityData& ad)
    {
        archive(
            cereal::make_nvp("base", ad.base),
            cereal::make_nvp("animationParams", ad.animationParams),
            cereal::make_nvp("vfx", ad.vfx));
    }
} // namespace sage