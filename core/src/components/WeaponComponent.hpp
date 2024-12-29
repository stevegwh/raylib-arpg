//
// Created by Steve Wheeler on 26/09/2024.
//

#pragma once

#include "common_types.hpp"
#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{
    struct WeaponComponent
    {
        entt::entity owner = entt::null;
        Matrix parentSocket{}; // Where the weapon is held by the owner in respect to their local model space
        std::string parentBoneName{};
        AssetID equipped{};
        // Would probably have a weapons' database with its model, icon, damage etc.
    };
} // namespace sage
