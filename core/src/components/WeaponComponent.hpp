//
// Created by Steve Wheeler on 26/09/2024.
//

#pragma once

#include "Event.hpp"

#include "common_types.hpp"
#include "entt/entt.hpp"
#include "raylib.h"
#include <memory>

namespace sage
{
    struct WeaponComponent
    {
        entt::entity owner = entt::null;
        Matrix parentSocket{}; // Where the weapon is held by the owner in respect to their local model space
        std::string parentBoneName{};
        AssetID equipped{};
        std::unique_ptr<Connection>
            animationFollowCnx; // Connection to the owner's animation movement for model to follow.
        // Would probably have a weapons' database with its model, icon, damage etc.
    };
} // namespace sage
