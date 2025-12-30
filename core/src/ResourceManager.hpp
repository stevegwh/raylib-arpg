//
// Created by Steve Wheeler on 16/07/2024.
//
#pragma once

#include "common_types.hpp"
#include "slib.hpp"

#include "magic_enum/magic_enum.hpp"
#include "raylib.h"
// #include "entt/entt.hpp"

// Below required
#include "cereal/archives/binary.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/unordered_map.hpp"
#include "cereal/types/vector.hpp"
#include "raylib-cereal.hpp"

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
        void FontLoadFromFile(const std::string& path);
        void ImageLoadFromFile(const std::string& path);
        void ImageLoadFromFile(const std::string& path, Image image);
        void ModelLoadFromFile(const std::string& path);
        void ModelAnimationLoadFromFile(const std::string& path);

      public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            return instance;
        }

        [[nodiscard]] Music GetMusic(const std::string& path);
        [[nodiscard]] Sound GetSFX(const std::string& path);
        [[nodiscard]] Shader ShaderLoad(const char* vsFileName, const char* fsFileName);
        [[nodiscard]] Texture TextureLoad(const std::string& path);
        [[nodiscard]] Texture TextureLoadFromImage(const std::string& name, Image image);
        Font FontLoad(const std::string& path);
        void ImageUnload(const std::string& key);
        [[nodiscard]] ImageSafe GetImage(const std::string& key);
        [[nodiscard]] ModelSafe GetModelCopy(const std::string& key);
        [[nodiscard]] ModelSafe GetModelDeepCopy(const std::string& key) const;
        [[nodiscard]] ModelAnimation* GetModelAnimation(const std::string& key, int* animsCount);
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
                // GetInstance().music,
                // GetInstance().sfx,
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
            // std::unordered_map<std::string, Music> _music;
            // std::unordered_map<std::string, Sound> _sfx;

            archive(
                _images,
                _modelCopies,
                _materialMap,
                // _music,
                // _sfx,
                animatedModelKeys,
                modelAnimCounts,
                modelAnimationsData);

            // WARNING: Does *not* account for overlapping keys (does nothing if key exists)
            images.merge(_images);
            modelCopies.merge(_modelCopies);
            materialMap.merge(_materialMap);
            // music.merge(_music);
            // sfx.merge(_sfx);

            // Merge leaves overlapping keys in the previous map. Ensure none are overlapping.
            assert(_images.empty());
            assert(_modelCopies.empty());
            assert(_materialMap.empty());

            for (auto& [key, model] : modelCopies)
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
                modelAnimations.emplace(animatedModelKeys[i], std::make_pair(animations, count));
            }
        }

        friend class ResourcePacker;
    };
} // namespace sage
