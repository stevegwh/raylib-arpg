//
// Created by Steve Wheeler on 26/12/2024.
//

#pragma once

namespace raylib
{
#include "raylib.h" // Declares module functions
}
using namespace raylib;

namespace sage::model
{
    Model LoadModel(const char* fileName);
} // namespace sage::model
