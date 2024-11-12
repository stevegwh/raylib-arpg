//
// Created by steve on 11/05/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{

    struct DialogComponent
    {
        entt::entity dialogTarget; // Who are you talking with
        Vector3 conversationPos;   // Where the other person stands
        std::string sentence;      // tmp
    };
} // namespace sage
