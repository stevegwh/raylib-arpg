//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "raylib.h"

#include <string>
#include <unordered_map>

namespace sage
{

    class ResourceManager
    {
        ResourceManager() = default;
        ~ResourceManager();

        std::unordered_map<std::string, Image> textureImages;
        std::unordered_map<std::string, Model> staticModels;
        std::unordered_map<std::string, Model> dynamicModels;
        std::unordered_map<std::string, std::pair<ModelAnimation*, int>> modelAnimations;
        std::unordered_map<std::string, char*> vertShaders;
        std::unordered_map<std::string, char*> fragShaders;
        
    public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            return instance;
        }

        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Image ImageLoad(const std::string& path);
        Model StaticModelLoad(const std::string& path);
        Model DynamicModelLoad(const std::string& path);
        ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
    };

} // namespace sage
