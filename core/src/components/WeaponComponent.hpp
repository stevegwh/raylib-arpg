//
// Created by Steve Wheeler on 26/09/2024.
//

#pragma once

#include "AssetID.hpp"

#include <entt/entt.hpp>

namespace sage
{
    struct WeaponComponent
    {
        entt::entity owner = entt::null;
        // Should add weapon transform here?
        AssetID equipped{};
        // Would probably have a weapons' database with its model, icon, damage etc.
    };
} // namespace sage
