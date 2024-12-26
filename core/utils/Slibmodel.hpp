//
// Created by Steve Wheeler on 26/12/2024.
//

#pragma once

#include "raylib.h"
#include <string>
#include <vector>

namespace sage
{
    struct ModelInfo;
    ModelInfo sgLoadModel(const char* fileName);
} // namespace sage
