//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "raylib.h"

#include <string>
#include <utility>

namespace sage
{
    struct Material
    {
        Texture2D diffuse{};
        const std::string path;
    };
}
