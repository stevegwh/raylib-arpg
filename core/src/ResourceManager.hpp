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
        static Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        static Image ImageLoad(const std::string& path);
        static Model StaticModelLoad(const std::string& path);
        static Model DynamicModelLoad(const std::string& path);
        static ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);
        ~ResourceManager();
    };

} // namespace sage
