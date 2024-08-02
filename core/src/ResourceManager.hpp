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
	static Model StaticModelLoad(const std::string& path);
	static Model DynamicModelLoad(const std::string& path);
	static ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);
    ~ResourceManager();
};

} // sage
