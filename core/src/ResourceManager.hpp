//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "raylib-cereal.hpp"
#include "raylib.h"
#include "slib.hpp"

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

namespace sage
{
    struct ModelResource
    {
        Model model;
        std::string materialKey;
        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(model, materialKey);
        }
    };

    class ResourceManager
    {
        ResourceManager();
        ~ResourceManager();

        std::unordered_map<std::string, Shader> shaders{};
        std::unordered_map<std::string, std::vector<Material>> modelMaterials{};
        std::unordered_map<std::string, Image> textureImages{}; // Change to "images"?
        std::unordered_map<std::string, Texture> textures{};    // Change to "freeTextures"?
        std::unordered_map<std::string, ModelResource> modelCopies{};
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

        template <class Archive>
        void save(Archive& archive) const
        {
            // Must already be initialised here.
            // TODO: Add "IsInitialised()" function
            std::vector<std::string> modelKeys;
            std::vector<ModelResource> modelData;

            std::vector<std::string> materialKeys;
            std::vector<std::vector<Material>> materialData;

            for (const auto& kv : modelMaterials)
            {
                materialKeys.push_back(kv.first);
                materialData.push_back(kv.second);
            }

            for (const auto& kv : modelCopies)
            {
                modelKeys.push_back(kv.first);
                modelData.push_back(kv.second);
            }

            archive(modelKeys, modelData, materialKeys, materialData);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<std::string> modelKeys;
            std::vector<ModelResource> modelData;

            std::vector<std::string> materialKeys;
            std::vector<std::vector<Material>> materialData;

            archive(modelKeys, modelData, materialKeys, materialData);

            for (int i = 0; i < materialKeys.size(); ++i)
            {
                GetInstance().modelMaterials.emplace(materialKeys[i], materialData[i]);
            }

            for (int i = 0; i < modelKeys.size(); ++i)
            {
                modelData[i].model.materials = modelMaterials.at(modelData[i].materialKey).data();
                GetInstance().modelCopies.emplace(modelKeys[i], modelData[i]);
            }
        }

        static void UnloadModelKeepMeshes(Model& model);
        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Texture TextureLoad(const std::string& path);
        void EmplaceModel(const std::string& path);
        void EmplaceModel(const std::string& modelKey, const std::string& materialKey, const std::string& path);
        ModelSafe LoadModelCopy(const std::string& path);
        ModelSafe LoadModelDeepCopy(const std::string& path);
        ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);
        void UnloadImages();
        void UnloadShaderFileText();

        void UnloadAll();
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
    };

} // namespace sage
