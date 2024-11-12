//
// Created by steve on 05/11/2024.
//

#pragma once

#include "AssetID.hpp"
#include <entt/entt.hpp>

namespace sage
{
    struct PartyMemberComponent
    {
        const entt::entity entity;
        AssetID portraitImage{};
        explicit PartyMemberComponent(entt::entity _entity) : entity(_entity){};
    };
} // namespace sage
