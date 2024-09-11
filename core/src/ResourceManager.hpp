//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "AssetID.hpp"
#include "magic_enum/magic_enum.hpp"
#include "raylib-cereal.hpp"
#include "raylib.h"
#include "slib.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/unordered_map.hpp"
#include "cereal/types/vector.hpp"

#include <entt/entt.hpp>
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
        std::unordered_map<AssetID, Image> images{};                             // Image (CPU) data
        std::unordered_map<AssetID, Texture> nonModelTextures{}; // Textures loaded outside of model loading
        std::unordered_map<AssetID, ModelCereal> modelCopies{};
        std::unordered_map<AssetID, std::pair<ModelAnimation*, int>> modelAnimations{};
        std::unordered_map<std::string, char*> vertShaderFileText{};
        std::unordered_map<std::string, char*> fragShaderFileText{};

        Shader gpuShaderLoad(const char* vs, const char* fs);
        static void deepCopyModel(const Model& oldModel, Model& newModel);
        static void deepCopyMesh(const Mesh& oldMesh, Mesh& mesh);

        static const std::string& getAssetPath(AssetID id);

      public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            return instance;
        }

        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Texture TextureLoad(AssetID id);
        void ImageUnload(AssetID id);
        ImageSafe GetImage(AssetID id);
        void ImageLoadFromFile(AssetID id);
        void ImageLoadFromFile(AssetID id, Image image);
        void ModelLoadFromFile(AssetID id);
        void ModelLoadFromFile(AssetID id, const std::string& materialKey);
        [[nodiscard]] ModelSafe GetModelCopy(AssetID id);
        [[nodiscard]] ModelSafe GetModelDeepCopy(AssetID id) const;
        void ModelAnimationLoadFromFile(AssetID id);
        ModelAnimation* GetModelAnimation(AssetID id, int* animsCount);
        void UnloadImages();
        void UnloadShaderFileText();

        void UnloadAll();
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<AssetID> imageKeys;
            std::vector<Image> imageData;

            std::vector<AssetID> modelKeys;
            std::vector<ModelCereal> modelData;

            std::vector<AssetID> animatedModelKeys;
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
            std::vector<AssetID> imageKeys;
            std::vector<Image> imageData;

            std::vector<AssetID> modelKeys;
            std::vector<ModelCereal> modelData;

            std::vector<AssetID> animatedModelKeys;
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
