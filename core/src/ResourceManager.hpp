//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "common_types.hpp"
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
    struct ModelInfo
    {
        Model model;
        std::vector<std::string>
            materialNames; // names of this mesh's materials (at the same index in model.materials)

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(model, materialNames);
        }
        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<std::string> _materialNames;
            archive(model, _materialNames);

            materialNames = _materialNames;
        }
    };

    class ResourceManager
    {
        ResourceManager();
        ~ResourceManager();

        std::unordered_map<std::string, Font> fonts{};
        std::unordered_map<std::string, Shader> shaders{};
        std::unordered_map<std::string, Material> materialMap;
        std::unordered_map<std::string, Image> images{};             // Image (CPU) data
        std::unordered_map<std::string, Texture> nonModelTextures{}; // Textures loaded outside of model loading
        std::unordered_map<std::string, ModelInfo> modelCopies{};
        std::unordered_map<std::string, std::pair<ModelAnimation*, int>> modelAnimations{};
        std::unordered_map<std::string, char*> vertShaderFileText{};
        std::unordered_map<std::string, char*> fragShaderFileText{};
        std::unordered_map<std::string, Music> music;
        std::unordered_map<std::string, Sound> sfx;

        Shader gpuShaderLoad(const char* vs, const char* fs);
        static void deepCopyModel(const Model& oldModel, Model& newModel);
        static void deepCopyMesh(const Mesh& oldMesh, Mesh& mesh);
        void init();

      public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            return instance;
        }

        Music MusicLoad(const std::string& path);
        Sound SFXLoad(const std::string& path);
        Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        Texture TextureLoad(const std::string& path);
        Texture TextureLoadFromImage(const std::string& name, Image image);
        Font FontLoad(const std::string& path);
        void ImageUnload(const std::string& key);
        ImageSafe GetImage(const std::string& key);
        void FontLoadFromFile(const std::string& path);
        void ImageLoadFromFile(const std::string& path);
        void ImageLoadFromFile(const std::string& path, Image image);
        void ModelLoadFromFile(const std::string& path);
        [[nodiscard]] ModelSafe GetModelCopy(const std::string& key);
        [[nodiscard]] ModelSafe GetModelDeepCopy(const std::string& key) const;
        void ModelAnimationLoadFromFile(const std::string& path);
        ModelAnimation* GetModelAnimation(const std::string& key, int* animsCount);
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
                GetInstance().materialMap,
                animatedModelKeys,
                modelAnimCounts,
                modelAnimationsData);

            //
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::vector<std::string> animatedModelKeys;
            std::vector<int> modelAnimCounts;
            std::vector<std::vector<ModelAnimation>> modelAnimationsData;

            // Copy necessary (I believe) so we can concat multiple calls of "load"
            std::unordered_map<std::string, Image> _images{};
            std::unordered_map<std::string, ModelInfo> _modelCopies{};
            std::unordered_map<std::string, Material> _materialMap;

            archive(_images, _modelCopies, _materialMap, animatedModelKeys, modelAnimCounts, modelAnimationsData);

            // WARNING: Does *not* account for overlapping keys (does nothing if key exists)
            images.merge(_images);
            modelCopies.merge(_modelCopies);
            materialMap.merge(_materialMap);

            for (auto& [key, model] : GetInstance().modelCopies)
            {
                for (unsigned int i = 0; i < model.materialNames.size(); ++i)
                {
                    const auto& mat = model.materialNames.at(i);
                    model.model.materials[i] = materialMap[mat];
                }
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
