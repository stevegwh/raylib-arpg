//
// Created by Steve Wheeler on 01/12/2024.
//

#pragma once

#include "abilities/AbilityData.hpp"
#include "components/Ability.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include <cereal/archives/json.hpp>
#include <magic_enum.hpp>
#include <magic_enum/magic_enum_flags.hpp>

namespace sage
{
    template <typename Archive>
    void serialize(Archive& archive, AnimationParams& anim)
    {
        archive(
            cereal::make_nvp("animEnum", anim.animEnum),
            cereal::make_nvp("animSpeed", anim.animSpeed),
            cereal::make_nvp("oneShot", anim.oneShot),
            cereal::make_nvp("animationDelay", anim.animationDelay));
    }
} // namespace sage

namespace lq
{
    template <typename Archive>
    void save(Archive& archive, AbilityData::BaseData const& bd)
    {
        std::vector<std::string> elementNames;
        std::vector<std::string> behaviourNames;
        std::vector<std::string> optionalBehaviourNames;

        // Use magic_enum to convert flags to strings
        for (auto flag : magic_enum::enum_values<AbilityElement>())
        {
            if (bd.HasElement(flag))
            {
                elementNames.emplace_back(magic_enum::enum_flags_name(flag));
            }
        }
        for (auto flag : magic_enum::enum_values<AbilityBehaviour>())
        {
            if (bd.HasBehaviour(flag))
            {
                behaviourNames.emplace_back(magic_enum::enum_flags_name(flag));
            }
        }
        for (auto flag : magic_enum::enum_values<AbilityBehaviourOptional>())
        {
            if (bd.HasOptionalBehaviour(flag))
            {
                optionalBehaviourNames.emplace_back(magic_enum::enum_flags_name(flag));
            }
        }

        archive(
            cereal::make_nvp("cooldownDuration", bd.cooldownDuration),
            cereal::make_nvp("baseDamage", bd.baseDamage),
            cereal::make_nvp("range", bd.range),
            cereal::make_nvp("radius", bd.radius),
            cereal::make_nvp("Elements", elementNames),
            cereal::make_nvp("Behaviours", behaviourNames),
            cereal::make_nvp("OptionalBehaviours", optionalBehaviourNames));
    }

    template <typename Archive>
    void load(Archive& archive, AbilityData::BaseData& bd)
    {
        std::vector<std::string> elementNames;
        std::vector<std::string> behaviourNames;
        std::vector<std::string> optionalBehaviourNames;

        archive(
            bd.cooldownDuration,
            bd.baseDamage,
            bd.range,
            bd.radius,
            elementNames,
            behaviourNames,
            optionalBehaviourNames);

        bd.elements = static_cast<AbilityElement>(0);
        bd.behaviour = static_cast<AbilityBehaviour>(0);
        bd.optional = static_cast<AbilityBehaviourOptional>(0);
        for (const auto& elementName : elementNames)
        {
            auto maybeFlag = magic_enum::enum_cast<AbilityElement>(elementName);
            if (maybeFlag)
            {
                bd.AddElement(*maybeFlag);
            }
        }
        for (const auto& flagName : behaviourNames)
        {
            auto maybeFlag = magic_enum::enum_cast<AbilityBehaviour>(flagName);
            if (maybeFlag)
            {
                bd.AddBehaviour(*maybeFlag);
            }
        }
        for (const auto& flagName : optionalBehaviourNames)
        {
            auto maybeFlag = magic_enum::enum_cast<AbilityBehaviourOptional>(flagName);
            if (maybeFlag)
            {
                bd.AddOptionalBehaviour(*maybeFlag);
            }
        }
    }

    template <typename Archive>
    void serialize(Archive& archive, AbilityData::VisualFXData& vfx)
    {
        archive(cereal::make_nvp("name", vfx.name));
    }

    template <class Archive>
    void save(Archive& archive, const AbilityData& ad)
    {
        archive(
            cereal::make_nvp("name", ad.name),
            cereal::make_nvp("description", ad.description),
            cereal::make_nvp("base", ad.base),
            cereal::make_nvp("animationParams", ad.animationParams),
            cereal::make_nvp("vfx", ad.vfx));
    }

    template <class Archive>
    void load(Archive& archive, AbilityData& ad)
    {
        archive(
            cereal::make_nvp("name", ad.name),
            cereal::make_nvp("description", ad.description),
            cereal::make_nvp("base", ad.base),
            cereal::make_nvp("animationParams", ad.animationParams),
            cereal::make_nvp("vfx", ad.vfx));
    }
} // namespace lq