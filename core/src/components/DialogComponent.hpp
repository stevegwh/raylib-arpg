//
// Created by steve on 11/05/2024.
//

#pragma once

#include "raylib.h"

namespace sage
{
    struct DialogComponent
    {
        Vector3 conversationPos; // Where the other person stands
        std::string sentence;    // tmp
    };
} // namespace sage
