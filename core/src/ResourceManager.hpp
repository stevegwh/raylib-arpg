//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "raylib.h"
#include "slib.hpp"

#include <string>
#include <unordered_map>

namespace sage
{

    class ResourceManager
    {
        ResourceManager() = default;
        ~ResourceManager();

        std::unordered_map<std::string, Shader> shaders{};
        std::unordered_map<std::string, Image> textureImages{};
        std::unordered_map<std::string, Texture> textures{};
        std::unordered_map<std::string, SafeModel> staticModels{};
        std::unordered_map<std::string, SafeModel> dynamicModels{};
        std::unordered_map<std::string, std::pair<ModelAnimation*, int>> modelAnimations{};
        std::unordered_map<std::string, char*> vertShaders{};
        std::unordered_map<std::string, char*> fragShaders{};

        Shader gpuShaderLoad(const std::string& vs, const std::string& fs);

      public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            return instance;
        }

        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Texture TextureLoad(const std::string& path);
        Image ImageLoad(const std::string& path);
        Model InstantiateModel(const std::string& path);
        static void DeepCopyMesh(const Mesh& oldMesh, Mesh& mesh);
        SafeModel DynamicModelLoad(const std::string& path);
        ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);

        void UnloadAll();
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
    };

} // namespace sage
