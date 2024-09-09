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
    struct ModelCereal
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
        std::unordered_map<std::string, std::vector<Material>> modelMaterials{}; // Shared model materials
        std::unordered_map<std::string, Image> images{};                         // Image (CPU) data
        std::unordered_map<std::string, Texture> nonModelTextures{}; // Textures loaded outside of model loading
        std::unordered_map<std::string, ModelCereal> modelCopies{};
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
            return instance;
        }

        static void UnloadModelKeepMeshes(Model& model);
        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Texture TextureLoad(const std::string& path);
        void EmplaceModel(const std::string& path);
        void EmplaceModel(const std::string& modelKey, const std::string& materialKey, const std::string& path);
        [[nodiscard]] ModelSafe LoadModelCopy(const std::string& path);
        [[nodiscard]] ModelSafe LoadModelDeepCopy(const std::string& path) const;
        ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);
        void UnloadImages();
        void UnloadShaderFileText();

        void UnloadAll();
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<std::string> modelKeys;
            std::vector<ModelCereal> modelData;

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
            std::vector<ModelCereal> modelData;

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
    };

} // namespace sage
