//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "common_types.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include <cereal/archives/json.hpp>

#include "enum_flag_operators.hpp"
#include <magic_enum.hpp>
#include <magic_enum/magic_enum_flags.hpp>

#include <string>

namespace lq
{
    enum class ItemFlags : unsigned int;
}
template <>
struct magic_enum::customize::enum_range<lq::ItemFlags>
{
    static constexpr bool is_flags = true;
};

namespace lq
{

    enum class ItemRarity
    {
        COMMON,
        UNCOMMON,
        RARE,
        EPIC,
        LEGENDARY
    };

    enum class ItemFlags : unsigned int
    {
        // Base item types
        WEAPON = 1 << 0,
        ARMOR = 1 << 1,
        POTION = 1 << 2,
        QUEST = 1 << 3,
        BOOK = 1 << 4,
        JUNK = 1 << 5,

        // Weapon types
        DAGGER = 1 << 8,
        SWORD = 1 << 9,
        BOW = 1 << 10,
        CROSSBOW = 1 << 11,
        STAFF = 1 << 12,
        WAND = 1 << 13,

        // Armor types
        HELMET = 1 << 16,
        AMULET = 1 << 17,
        CHEST = 1 << 18,
        BELT = 1 << 19,
        BOOTS = 1 << 20,
        RING = 1 << 21,
        LEGS = 1 << 22,
        ARMS = 1 << 23,

        // Weapon properties
        TWO_HANDED = 1 << 24,
        MAIN_HAND_ONLY = 1 << 25,
        CAN_BACKSTAB = 1 << 26,
    };

    template <>
    struct EnableBitMaskOperators<ItemFlags>
    {
        static const bool enable = true;
    };

    struct ItemComponent
    {
        static constexpr float MAX_ITEM_DROP_RANGE = 40.0f;
        std::string name;
        std::string localizedName;
        std::string description;
        ItemRarity rarity = static_cast<ItemRarity>(0);
        ItemFlags flags = static_cast<ItemFlags>(0);
        AssetID icon;
        AssetID model;

        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<std::string> flagNames;

            // Use magic_enum to convert flags to strings
            for (auto flag : magic_enum::enum_values<ItemFlags>())
            {
                if (HasFlag(flag))
                {
                    flagNames.emplace_back(magic_enum::enum_flags_name(flag));
                }
            }

            archive(
                cereal::make_nvp("Name", name),
                cereal::make_nvp("LocalizedName", localizedName),
                cereal::make_nvp("Description", description),
                cereal::make_nvp("Rarity", std::string(magic_enum::enum_name(rarity))),
                cereal::make_nvp("Flags", flagNames),
                cereal::make_nvp("Icon", icon),
                cereal::make_nvp("Model", model));
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<std::string> flagNames;
            std::string _rarity;

            archive(name, localizedName, description, _rarity, flagNames, icon, model);

            flags = static_cast<ItemFlags>(0);

            for (const auto& flagName : flagNames)
            {
                auto maybeFlag = magic_enum::enum_cast<ItemFlags>(flagName);
                if (maybeFlag)
                {
                    AddFlag(*maybeFlag);
                }
            }

            rarity = magic_enum::enum_cast<ItemRarity>(_rarity).value();
        }

        // Helper methods for flags
        void AddFlag(ItemFlags flag)
        {
            flags |= flag;
        }

        void RemoveFlag(ItemFlags flag)
        {
            flags = static_cast<ItemFlags>(static_cast<unsigned int>(flags) & ~static_cast<unsigned int>(flag));
        }

        void RemoveAllFlags()
        {
            flags = static_cast<ItemFlags>(0);
        }

        [[nodiscard]] bool HasFlag(ItemFlags flag) const
        {
            return (static_cast<unsigned int>(flags) & static_cast<unsigned int>(flag)) != 0;
        }

        [[nodiscard]] bool HasAllFlags(ItemFlags flagSet) const
        {
            return (static_cast<unsigned int>(flags) & static_cast<unsigned int>(flagSet)) ==
                   static_cast<unsigned int>(flagSet);
        }

        [[nodiscard]] bool HasAnyFlag(ItemFlags flagSet) const
        {
            return (static_cast<unsigned int>(flags) & static_cast<unsigned int>(flagSet)) != 0;
        }
    };

}; // namespace lq
