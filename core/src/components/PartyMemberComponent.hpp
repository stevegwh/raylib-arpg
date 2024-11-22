//
// Created by steve on 05/11/2024.
//

#pragma once

#include "AssetID.hpp"
#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{
    struct PartyMemberComponent
    {
        const entt::entity entity;
        RenderTexture portraitImg{};
        AssetID portraitImage{};
        explicit PartyMemberComponent(entt::entity _entity) : entity(_entity)
        {
        }
    };
} // namespace sage
