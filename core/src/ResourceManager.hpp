//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "raylib.h"
#include "slib.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sage
{

    class ResourceManager
    {
        ResourceManager();
        ~ResourceManager();

        std::unordered_map<std::string, Shader> shaders{};
        std::unordered_map<std::string, Image> textureImages{};
        std::unordered_map<std::string, Texture> textures{};
        std::unordered_map<std::string, std::unique_ptr<ModelSafe>> modelCopies{};
        std::unordered_map<std::string, std::pair<ModelAnimation*, int>> modelAnimations{};
        std::unordered_map<std::string, char*> vertShaderFileText{};
        std::unordered_map<std::string, char*> fragShaderFileText{};

        Shader gpuShaderLoad(const char* vs, const char* fs);
        Image imageLoad(const std::string& path);

        static void deepCopyModel(const Model& oldModel, Model& newModel);
        static void deepCopyMesh(const Mesh& oldMesh, Mesh& mesh);

      public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            // shaders.emplace("DEFAULT", LoadMaterialDefault().shader);
            return instance;
        }
        static void UnloadModelKeepMeshes(Model& model);
        static std::vector<entt::entity> UnpackOBJMap(
            entt::registry* registry, MaterialPaths material_paths, const std::string& mapPath);
        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Texture TextureLoad(const std::string& path);
        void EmplaceModel(const std::string& path);
        std::unique_ptr<ModelSafe> LoadModelCopy(const std::string& path);
        std::unique_ptr<ModelSafe> LoadModelDeepCopy(const std::string& path);
        ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);
        void UnloadImages();
        void UnloadShaderFileText();

        void UnloadAll();
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
    };

} // namespace sage
