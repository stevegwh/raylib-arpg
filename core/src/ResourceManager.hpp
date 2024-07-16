//
// Created by Steve Wheeler on 16/07/2024.
//

#pragma once

#include <string>

#include "raylib.h"

namespace sage
{

class ResourceManager
{
public:
    static Image LoadTexture(const std::string& path);
    ~ResourceManager();
};

} // sage
