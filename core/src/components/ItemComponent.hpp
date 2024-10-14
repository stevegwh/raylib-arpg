//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "AssetID.hpp"

#include <string>

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
    struct ItemComponent
    {
        std::string name;
        std::string description;
        ItemRarity rarity = ItemRarity::COMMON;
        AssetID icon;
        AssetID model;
    };
}; // namespace sage
