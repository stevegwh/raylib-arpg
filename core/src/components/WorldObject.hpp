//
// Created by Steve Wheeler on 23/02/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    struct WorldObject
    {
        entt::entity parent{};
        std::vector<entt::entity> children;
    };
}

