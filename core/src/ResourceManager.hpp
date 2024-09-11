//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "raylib-cereal.hpp"
#include "raylib.h"
#include "slib.hpp"

#include <entt/entt.hpp>
#include <external/glad.h>
#include <string>
#include <unordered_map>

namespace sage
{
    struct ModelCereal
    {
        Model model;
        // animations
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
        static void deepCopyModel(const Model& oldModel, Model& newModel);
        static void deepCopyMesh(const Mesh& oldMesh, Mesh& mesh);

      public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            return instance;
        }

        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Texture TextureLoad(const std::string& path);
        void ImageUnload(const std::string& path);
        ImageSafe GetImage(const std::string& path);
        void EmplaceImage(const std::string& path);
        void EmplaceImage(const std::string& key, Image image);
        void EmplaceImage(const std::string& key, const std::string& path);
        void EmplaceModel(const std::string& path);
        void EmplaceModel(const std::string& modelKey, const std::string& materialKey, const std::string& path);
        [[nodiscard]] ModelSafe LoadModelCopy(const std::string& path);
        [[nodiscard]] ModelSafe LoadModelDeepCopy(const std::string& path) const;
        void EmplaceModelAnimation(const std::string& path);
        ModelAnimation* ModelAnimationLoad(const std::string& path, int* animsCount);
        void UnloadImages();
        void UnloadShaderFileText();

        std::string GetAssetPath();

        void UnloadAll();
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<std::string> imageKeys;
            std::vector<Image> imageData;

            std::vector<std::string> modelKeys;
            std::vector<ModelCereal> modelData;

            std::vector<std::string> animatedModelKeys;
            std::vector<int> modelAnimCounts;
            std::vector<std::vector<ModelAnimation>> modelAnimationsData;

            std::vector<std::string> materialKeys;
            std::vector<std::vector<Material>> materialData;

            for (const auto& [imageKey, image] : images)
            {
                imageKeys.push_back(imageKey);
                imageData.push_back(image);
            }

            for (const auto& kv : modelMaterials)
            {
                materialKeys.push_back(kv.first);
                materialData.push_back(kv.second);
            }

            for (const auto& [modelKey, modelCereal] : modelCopies)
            {
                modelKeys.push_back(modelKey);
                modelData.push_back(modelCereal);

                if (modelAnimations.contains(modelKey))
                {
                    animatedModelKeys.push_back(modelKey);
                    const auto& [modelAnims, count] = modelAnimations.at(modelKey);
                    modelAnimCounts.push_back(count);
                    modelAnimationsData.emplace_back(modelAnims, modelAnims + count);
                }
            }

            archive(
                imageKeys,
                imageData,
                modelKeys,
                modelData,
                materialKeys,
                materialData,
                animatedModelKeys,
                modelAnimCounts,
                modelAnimationsData);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<std::string> imageKeys;
            std::vector<Image> imageData;

            std::vector<std::string> modelKeys;
            std::vector<ModelCereal> modelData;

            std::vector<std::string> animatedModelKeys;
            std::vector<int> modelAnimCounts;
            std::vector<std::vector<ModelAnimation>> modelAnimationsData;

            std::vector<std::string> materialKeys;
            std::vector<std::vector<Material>> materialData;

            archive(
                imageKeys,
                imageData,
                modelKeys,
                modelData,
                materialKeys,
                materialData,
                animatedModelKeys,
                modelAnimCounts,
                modelAnimationsData);

            for (int i = 0; i < imageKeys.size(); ++i)
            {
                GetInstance().images.emplace(imageKeys[i], imageData[i]);
            }

            for (int i = 0; i < materialKeys.size(); ++i)
            {
                GetInstance().modelMaterials.emplace(materialKeys[i], materialData[i]);
            }

            for (int i = 0; i < modelKeys.size(); ++i)
            {
                modelData[i].model.materials = modelMaterials.at(modelData[i].materialKey).data();
                GetInstance().modelCopies.emplace(modelKeys[i], modelData[i]);
            }

            for (int i = 0; i < animatedModelKeys.size(); ++i)
            {
                auto count = modelAnimCounts[i];
                auto* animations = static_cast<ModelAnimation*>(RL_MALLOC(count * sizeof(ModelAnimation)));
                std::memcpy(animations, modelAnimationsData[i].data(), count * sizeof(ModelAnimation));
                GetInstance().modelAnimations.emplace(animatedModelKeys[i], std::make_pair(animations, count));
            }
        }
    };

} // namespace sage
