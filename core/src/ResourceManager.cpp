//
// Created by Steve Wheeler on 16/07/2024.
//

#include "ResourceManager.hpp"

#include "AssetManager.hpp"
#include "components/Renderable.hpp"
#include "Slibmodel.hpp"

#include "raylib/src/config.h"
#include "raymath.h"
#include "rlgl.h"
#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_NONE

#include <stb_include.h>

#include <cstring>
#include <sstream>
#include <unordered_map>

namespace sage
{

    Shader ResourceManager::gpuShaderLoad(const char* vs, const char* fs)
    {
        std::string vs_str = vs == nullptr ? "" : std::string(vs);
        std::string fs_str = fs == nullptr ? "" : std::string(fs);
        std::string concat = vs_str + fs_str;

        if (!shaders.contains(concat))
        {
            shaders[concat] = LoadShaderFromMemory(vs, fs);
        }

        return shaders[concat];
    }

    void ResourceManager::deepCopyModel(const Model& oldModel, Model& model)
    {
        model.meshCount = oldModel.meshCount;
        model.materialCount = oldModel.materialCount;
        model.boneCount = oldModel.boneCount;
        model.meshes = static_cast<Mesh*>(RL_CALLOC(model.meshCount, sizeof(Mesh)));
        model.bones = static_cast<BoneInfo*>(RL_MALLOC(model.boneCount * sizeof(BoneInfo)));
        model.bindPose = static_cast<Transform*>(RL_MALLOC(model.boneCount * sizeof(Transform)));

        for (size_t i = 0; i < model.meshCount; ++i)
        {
            deepCopyMesh(oldModel.meshes[i], model.meshes[i]);
        }

        if (model.materialCount == 0)
        {
            // TRACELOG(LOG_WARNING, "MATERIAL: [%s] Failed to load model material data,
            // default to white material", fileName);

            model.materialCount = 1;
            model.materials = static_cast<Material*>(RL_CALLOC(model.materialCount, sizeof(Material)));
            model.materials[0] = LoadMaterialDefault();

            if (model.meshMaterial == nullptr)
                model.meshMaterial = static_cast<int*>(RL_CALLOC(model.meshCount, sizeof(int)));
        }
        else
        {
            model.materials = static_cast<Material*>(RL_CALLOC(model.materialCount, sizeof(Material)));
            model.meshMaterial = static_cast<int*>(RL_CALLOC(model.meshCount, sizeof(int)));

            for (size_t i = 0; i < model.materialCount; ++i)
            {
                model.materials[i] = oldModel.materials[i];
                model.materials[i].shader = oldModel.materials[i].shader;

                // // NB: If wanting to deep copy the shader, you MUST deallocate shader.locs in the destructor.
                // // Deep copy shader locs
                // model.materials[i].shader.locs = (int*)RL_MALLOC(RL_MAX_SHADER_LOCATIONS * sizeof(int));
                // memcpy(
                //     model.materials[i].shader.locs,
                //     oldModel.materials[i].shader.locs,
                //     RL_MAX_SHADER_LOCATIONS * sizeof(int));

                // Shallow copy shader locs
                model.materials[i].shader.locs = oldModel.materials[i].shader.locs;

                // Deep copy maps
                model.materials[i].maps =
                    static_cast<MaterialMap*>(RL_MALLOC(MAX_MATERIAL_MAPS * sizeof(MaterialMap)));
                std::memcpy(
                    model.materials[i].maps, oldModel.materials[i].maps, MAX_MATERIAL_MAPS * sizeof(MaterialMap));

                // TODO: Does not set a new texture id, causing issues when freeing it in ModelSafe
                // See "LoadTextureFromImage" and raylib-cereal.
                // Consider if you really want a new texture per model or if "deep copy" is unnecessary
                // I think a shallow copy but new animVertices and animNormals would be fine.

                // Copy params
                std::memcpy(model.materials[i].params, oldModel.materials[i].params, 4 * sizeof(float));
            }

            for (size_t i = 0; i < model.meshCount; ++i)
            {
                model.meshMaterial[i] = oldModel.meshMaterial[i];
            }
        }

        for (size_t i = 0; i < model.boneCount; ++i)
        {
            model.bones[i] = oldModel.bones[i];
        }
        for (size_t i = 0; i < model.boneCount; ++i)
        {
            model.bindPose[i] = oldModel.bindPose[i];
        }

        // Below taken from raylib's LoadModel().
        model.transform = MatrixIdentity();
        if ((model.meshCount != 0) && (model.meshes != nullptr))
        {
            // Upload vertex data to GPU (static meshes)
            for (int i = 0; i < model.meshCount; i++)
            {
                UploadMesh(&model.meshes[i], false);
            }
        }
        // else TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load model mesh(es) data",
        // "Cereal Model Import");
    }

    void ResourceManager::deepCopyMesh(const Mesh& oldMesh, Mesh& mesh)
    {
        mesh.vertexCount = oldMesh.vertexCount;
        mesh.triangleCount = oldMesh.triangleCount;

        // Copy basic vertex data
        mesh.vertices = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
        memcpy(mesh.vertices, oldMesh.vertices, mesh.vertexCount * 3 * sizeof(float));

        if (oldMesh.texcoords)
        {
            mesh.texcoords = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 2 * sizeof(float)));
            memcpy(mesh.texcoords, oldMesh.texcoords, mesh.vertexCount * 2 * sizeof(float));
        }
        if (oldMesh.texcoords2)
        {
            mesh.texcoords2 = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 2 * sizeof(float)));
            memcpy(mesh.texcoords2, oldMesh.texcoords2, mesh.vertexCount * 2 * sizeof(float));
        }
        if (oldMesh.normals)
        {
            mesh.normals = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
            memcpy(mesh.normals, oldMesh.normals, mesh.vertexCount * 3 * sizeof(float));
        }
        if (oldMesh.tangents)
        {
            mesh.tangents = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(float)));
            memcpy(mesh.tangents, oldMesh.tangents, mesh.vertexCount * 4 * sizeof(float));
        }
        if (oldMesh.colors)
        {
            mesh.colors = static_cast<unsigned char*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char)));
            memcpy(mesh.colors, oldMesh.colors, mesh.vertexCount * 4 * sizeof(unsigned char));
        }
        if (oldMesh.indices)
        {
            mesh.indices =
                static_cast<unsigned short*>(RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short)));
            memcpy(mesh.indices, oldMesh.indices, mesh.triangleCount * 3 * sizeof(unsigned short));
        }

        if (oldMesh.animVertices)
        {
            mesh.animVertices = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
            memcpy(mesh.animVertices, oldMesh.vertices, mesh.vertexCount * 3 * sizeof(float));
        }
        if (oldMesh.animNormals)
        {
            mesh.animNormals = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
            memcpy(mesh.animNormals, oldMesh.normals, mesh.vertexCount * 3 * sizeof(float));
        }
        if (oldMesh.boneIds)
        {
            mesh.boneIds = static_cast<unsigned char*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char)));
            memcpy(mesh.boneIds, oldMesh.boneIds, mesh.vertexCount * 4 * sizeof(unsigned char));
        }
        if (oldMesh.boneWeights)
        {
            mesh.boneWeights = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(float)));
            memcpy(mesh.boneWeights, oldMesh.boneWeights, mesh.vertexCount * 4 * sizeof(float));
        }
        mesh.boneCount = oldMesh.boneCount;
        if (oldMesh.boneMatrices)
        {
            mesh.boneMatrices = static_cast<Matrix*>(RL_CALLOC(mesh.boneCount, sizeof(Matrix)));
            for (int j = 0; j < mesh.boneCount; j++)
            {
                mesh.boneMatrices[j] = MatrixIdentity();
                // Gets updated per animation, no need to copy info over.
            }
        }

        mesh.vaoId = 0; // Default value (ensures it gets uploaded to gpu)
    }

    /*
    * @brief Stores the shader's text file in memory, saving on reading the file multiple
    times.
     *
     * @param vShaderStr
     * @param fShaderStr
     * @return Shader
    */
    Shader ResourceManager::ShaderLoad(const char* vsFileName, const char* fsFileName)
    {
        if (vsFileName == nullptr && fsFileName == nullptr || (vsFileName != nullptr && !FileExists(vsFileName) &&
                                                               (fsFileName != nullptr && !FileExists(fsFileName))))
        {
            std::cout << "WARNING: Both files nullptr or do not exist. Loading default shader. \n";
            return shaders["DEFAULT"];
        }

        char* vShaderStr = nullptr;
        char* fShaderStr = nullptr;

        const char* SHADER_INCLUDE_PATH = "resources/shaders/custom/include";

        if (vsFileName != nullptr)
        {
            if (!vertShaderFileText.contains(vsFileName))
            {
                assert(FileExists(vsFileName));
                // Load and preprocess vertex shader with stb_include
                char* vertexSource = LoadFileText(vsFileName);
                char* preprocessed =
                    stb_include_string(vertexSource, nullptr, (char*)SHADER_INCLUDE_PATH, nullptr, nullptr);
                free(vertexSource);
                vertShaderFileText[vsFileName] = preprocessed;
            }
            vShaderStr = vertShaderFileText[vsFileName];
        }

        if (fsFileName != nullptr)
        {
            if (!fragShaderFileText.contains(fsFileName))
            {
                assert(FileExists(fsFileName));
                // Load and preprocess fragment shader with stb_include
                char* fragmentSource = LoadFileText(fsFileName);
                char* preprocessed =
                    stb_include_string(fragmentSource, nullptr, (char*)SHADER_INCLUDE_PATH, nullptr, nullptr);
                free(fragmentSource);
                fragShaderFileText[fsFileName] = preprocessed;
            }
            fShaderStr = fragShaderFileText[fsFileName];
        }

        return gpuShaderLoad(vShaderStr, fShaderStr);
    }

    Texture ResourceManager::TextureLoad(const std::string& path)
    {
        auto key = AssetManager::GetInstance().TryGetAssetPath(path);
        if (!nonModelTextures.contains(key))
        {
            assert(images.contains(key));
            nonModelTextures[key] = LoadTextureFromImage(images[key]);
        }
        return nonModelTextures[key];
    }

    Texture ResourceManager::TextureLoad(const std::string& fileName, const std::string& path)
    {
        if (!nonModelTextures.contains(fileName))
        {
            if (!images.contains(fileName))
            {
                images.emplace(fileName, LoadImage(path.c_str()));
            }
            nonModelTextures[path] = LoadTextureFromImage(images[fileName]);
        }
        return nonModelTextures[path];
    }

    Texture ResourceManager::TextureLoadFromImage(const std::string& name, Image image)
    {
        if (!images.contains(name))
        {
            images.emplace(name, image);
            nonModelTextures[name] = LoadTextureFromImage(images[name]);
        }
        return nonModelTextures[name];
    }

    Font ResourceManager::FontLoad(const std::string& path)
    {
        // TODO: asset id
        assert(fonts.contains(path));
        return fonts[path];
    }

    void ResourceManager::ImageUnload(const std::string& id)
    {
        auto path = AssetManager::GetInstance().TryGetAssetPath(id);
        if (images.contains(path))
        {
            UnloadImage(images.at(path));
            images.erase(path);
        }
    }

    ImageSafe ResourceManager::GetImage(const std::string& path)
    {
        auto key = AssetManager::GetInstance().TryGetAssetPath(path);
        assert(images.contains(key));
        return ImageSafe(images[key], false);
    }

    void ResourceManager::FontLoadFromFile(const std::string& path)
    {
        assert(FileExists(path.c_str()));
        if (!fonts.contains(path))
        {
            fonts[path] = LoadFont(path.c_str());
        }
    }

    void ResourceManager::ImageLoadFromFile(const std::string& path)
    {
        auto key = AssetManager::GetInstance().TryGetAssetPath(path);
        assert(FileExists(key.c_str()));
        if (!images.contains(key))
        {
            images[key] = LoadImage(key.c_str());
        }
    }

    void ResourceManager::ImageLoadFromFile(const std::string& path, Image image)
    {
        auto key = AssetManager::GetInstance().TryGetAssetPath(path);
        if (!images.contains(key))
        {
            images[key] = image;
            image = {};
        }
    }

    void ResourceManager::ModelLoadFromFile(const std::string& path)
    {
        auto key = AssetManager::GetInstance().TryGetAssetPath(path);
        if (!modelCopies.contains(key))
        {
            assert(FileExists(key.c_str()));

            auto modelInfo = sgLoadModel(key.c_str());

            for (unsigned int i = 0; i < modelInfo.materialNames.size(); ++i)
            {
                const auto& mat = modelInfo.materialNames[i];
                if (!materialMap.contains(mat))
                {
                    materialMap.emplace(mat, modelInfo.model.materials[i]);
                }
                else
                {
                    UnloadMaterial(modelInfo.model.materials[i]);
                }
                modelInfo.model.materials[i] = materialMap.at(mat);
            }

            modelCopies.emplace(key, std::move(modelInfo));
        }
    }

    /**
     * @brief Returns a shallow copy of the loaded model
     * NB: Caller should not free the memory.
     * @param path
     * @return Model
     */

    ModelSafe ResourceManager::GetModelCopy(const std::string& path)
    {
        auto key = AssetManager::GetInstance().TryGetAssetPath(path);
        assert(modelCopies.contains(key));
        ModelSafe modelsafe(modelCopies.at(key).model, false);
        modelsafe.SetKey(key);
        return std::move(modelsafe);
    }

    /**
     * @brief Creates a deep copy of the loaded model. Cuts down model loading times as
     * it's faster copying buffers rather than reading/parsing model files.
     * @param path
     * @return
     */
    ModelSafe ResourceManager::GetModelDeepCopy(const std::string& id) const
    {
        // TODO: Unsure if deep copy is ever really necessary.
        // For animated models, I believe all that's needed is to shallow copy the mesh minus
        // animVertices/animNormals. So, allocate memory for new meshes, shallow copy the majority of its data, but
        // allocate and point to new animVertices/animNormals arrays
        const auto& path = AssetManager::GetInstance().TryGetAssetPath(id);
        assert(modelCopies.contains(path));
        Model model = modelCopies.at(path).model;
        deepCopyModel(modelCopies.at(path).model, model);
        return ModelSafe(model);
    }

    void ResourceManager::ModelAnimationLoadFromFile(const std::string& id)
    {
        const auto& path = AssetManager::GetInstance().TryGetAssetPath(id);
        if (!modelAnimations.contains(path))
        {
            int animsCount;
            auto animations = LoadModelAnimations(path.c_str(), &animsCount);
            if (animations == nullptr)
            {
                std::cout << "ResourceManager: Model does not contain animation data, or was unable to be loaded. "
                             "Aborting... \n";
                return;
            }
            modelAnimations[path] = std::make_pair(animations, animsCount);
        }
    }

    ModelAnimation* ResourceManager::GetModelAnimation(const std::string& id, int* animsCount)
    {
        const auto& path = AssetManager::GetInstance().TryGetAssetPath(id);
        assert(modelAnimations.contains(path));
        const auto& pair = modelAnimations[path];
        *animsCount = pair.second;
        return pair.first;
    }

    void ResourceManager::UnloadImages()
    {
        for (const auto& [key, image] : images)
        {
            UnloadImage(image);
        }
        images.clear();
    }

    void ResourceManager::UnloadShaderFileText()
    {
        for (const auto& [key, vs] : vertShaderFileText)
        {
            UnloadFileText(vs);
        }
        for (const auto& [key, fs] : fragShaderFileText)
        {
            UnloadFileText(fs);
        }
        vertShaderFileText.clear();
        fragShaderFileText.clear();
    }

    void sgUnloadModel(Model model)
    {
        // Unload meshes
        for (int i = 0; i < model.meshCount; i++)
            UnloadMesh(model.meshes[i]);

        // Unload arrays
        RL_FREE(model.meshes);
        RL_FREE(model.meshMaterial);

        // Unload animation data
        RL_FREE(model.bones);
        RL_FREE(model.bindPose);

        TRACELOG(LOG_INFO, "MODEL: Unloaded model (and meshes) from RAM and VRAM");
    }

    void ResourceManager::UnloadAll()
    {
        for (auto& [key, mat] : materialMap)
        {
            for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
            {
                if (mat.maps[i].texture.id != rlGetTextureIdDefault()) rlUnloadTexture(mat.maps[i].texture.id);
            }
            RL_FREE(mat.maps);
        }
        for (auto& [path, model] : modelCopies)
        {
            sgUnloadModel(model.model);
        }
        for (const auto& [key, tex] : nonModelTextures)
        {
            UnloadTexture(tex);
        }
        for (const auto& [key, image] : images)
        {
            UnloadImage(image);
        }
        for (const auto& [key, p] : modelAnimations)
        {
            UnloadModelAnimations(p.first, p.second);
        }
        for (const auto& [key, shader] : shaders)
        {
            UnloadShader(shader);
        }
        for (const auto& [key, text] : vertShaderFileText)
        {
            UnloadFileText(text);
        }
        for (const auto& [key, text] : fragShaderFileText)
        {
            UnloadFileText(text);
        }
        modelCopies.clear();
        nonModelTextures.clear();
        images.clear();
        modelAnimations.clear();
        shaders.clear();
        vertShaderFileText.clear();
        fragShaderFileText.clear();
    }

    void ResourceManager::Reset()
    {
        UnloadAll();
        init();
    }

    void ResourceManager::init()
    {
        Shader shader;
        shader.id = rlGetShaderIdDefault();
        shader.locs = rlGetShaderLocsDefault();
        shaders.emplace("DEFAULT", shader);
    }

    ResourceManager::~ResourceManager()
    {
        UnloadAll();
    }

    ResourceManager::ResourceManager()
    {
        init();
    }
} // namespace sage