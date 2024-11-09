//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "AssetID.hpp"

#include <string>
#include <type_traits>

namespace sage
{
    enum class ItemRarity
    {
        COMMON,
        UNCOMMON,
        RARE,
        EPIC,
        LEGENDARY
    };

    namespace ItemFlags
    {
        // Base item types
        static constexpr unsigned int WEAPON = 1u << 0;
        static constexpr unsigned int ARMOR = 1u << 1;
        static constexpr unsigned int POTION = 1u << 2;
        static constexpr unsigned int QUEST = 1u << 3;
        static constexpr unsigned int BOOK = 1u << 4;
        static constexpr unsigned int JUNK = 1u << 5;

        // Weapon types (offset by 8 bits)
        static constexpr unsigned int DAGGER = 1u << 8;
        static constexpr unsigned int SWORD = 1u << 9;
        static constexpr unsigned int BOW = 1u << 10;
        static constexpr unsigned int CROSSBOW = 1u << 11;
        static constexpr unsigned int STAFF = 1u << 12;
        static constexpr unsigned int WAND = 1u << 13;

        // Armor types (offset by 16 bits)
        static constexpr unsigned int HELMET = 1u << 16;
        static constexpr unsigned int AMULET = 1u << 17;
        static constexpr unsigned int CHEST = 1u << 18;
        static constexpr unsigned int BELT = 1u << 19;
        static constexpr unsigned int BOOTS = 1u << 20;
        static constexpr unsigned int RING = 1u << 21;

        // Weapon properties (offset by 24 bits)
        static constexpr unsigned int TWO_HANDED = 1u << 24;
        static constexpr unsigned int MAIN_HAND_ONLY = 1u << 25;
        static constexpr unsigned int CAN_BACKSTAB = 1u << 26;

        // Helpful masks for checking categories
        static constexpr unsigned int WEAPON_TYPE_MASK = 0x0000FF00;
        static constexpr unsigned int ARMOR_TYPE_MASK = 0x00FF0000;
        static constexpr unsigned int WEAPON_PROPS_MASK = 0xFF000000;
    } // namespace ItemFlags

    // Damage type?

    struct ItemComponent
    {
        static constexpr float MAX_ITEM_DROP_RANGE = 40.0f;
        std::string name;
        std::string description;
        ItemRarity rarity = ItemRarity::COMMON;
        unsigned int flags = ItemFlags::JUNK;
        AssetID icon;
        AssetID model;

        [[nodiscard]] bool HasFlag(unsigned int flag) const
        {
            return (flags & flag) == flag;
        }
        void AddFlag(unsigned int flag)
        {
            flags |= flag;
        }
        void RemoveFlag(unsigned int flag)
        {
            flags &= ~flag;
        }
    };
}; // namespace sage
