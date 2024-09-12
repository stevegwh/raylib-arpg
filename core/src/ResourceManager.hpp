//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "AssetID.hpp"
#include "magic_enum/magic_enum.hpp"
// #include "raylib-cereal.hpp"
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
        void init();
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
        void ImageLoadFromFile(const std::string& path, Image image);
        void ModelLoadFromFile(AssetID id);
        void ModelLoadFromFile(AssetID id, const std::string& materialKey);
        void ModelLoadFromFile(const std::string& path);
        void ModelLoadFromFile(const std::string& path, const std::string& materialKey);
        [[nodiscard]] ModelSafe GetModelCopy(const std::string& id);
        [[nodiscard]] ModelSafe GetModelCopy(AssetID id);
        [[nodiscard]] ModelSafe GetModelDeepCopy(AssetID id) const;
        void ModelAnimationLoadFromFile(AssetID id);
        ModelAnimation* GetModelAnimation(AssetID id, int* animsCount);
        void UnloadImages();
        void UnloadShaderFileText();

        void UnloadAll();
        void Reset();
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<std::string> animatedModelKeys;
            std::vector<int> modelAnimCounts;
            std::vector<std::vector<ModelAnimation>> modelAnimationsData;

            for (const auto& [key, p] : GetInstance().modelAnimations)
            {
                const auto& [modelAnims, count] = p;
                animatedModelKeys.push_back(key);
                modelAnimCounts.push_back(count);
                modelAnimationsData.emplace_back(modelAnims, modelAnims + count);
            }

            archive(
                GetInstance().images,
                GetInstance().modelCopies,
                GetInstance().modelMaterials,
                animatedModelKeys,
                modelAnimCounts,
                modelAnimationsData);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<std::string> animatedModelKeys;
            std::vector<int> modelAnimCounts;
            std::vector<std::vector<ModelAnimation>> modelAnimationsData;

            // Copy necessary (I believe) so we can concat multiple calls of "load"
            std::unordered_map<std::string, Image> _images{};
            std::unordered_map<std::string, ModelCereal> _modelCopies{};
            std::unordered_map<std::string, std::vector<Material>> _modelMaterials{};

            archive(
                _images, _modelCopies, _modelMaterials, animatedModelKeys, modelAnimCounts, modelAnimationsData);

            // WARNING: Does *not* account for overlapping keys (does nothing if key exists)
            images.insert(_images.begin(), _images.end());
            modelCopies.insert(_modelCopies.begin(), _modelCopies.end());
            modelMaterials.insert(_modelMaterials.begin(), _modelMaterials.end());

            for (auto& [key, model] : GetInstance().modelCopies)
            {
                model.model.materials = modelMaterials.at(model.materialKey).data();
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
