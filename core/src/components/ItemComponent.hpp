//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "AssetID.hpp"

#include <string>

namespace sage
{
    struct ItemComponent
    {
        std::string name;
        std::string description;
        AssetID icon;
        AssetID model;
    };
}; // namespace sage
