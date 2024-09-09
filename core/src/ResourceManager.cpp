//
// Created by Steve Wheeler on 16/07/2024.
//

#include "ResourceManager.hpp"

#include "components/Renderable.hpp"

#include "raylib/src/config.h"
#include "raymath.h"
#include "rlgl.h"

#include <cstring>
#include <sstream>
#include <unordered_map>
#include <utils.h>

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

    Image ResourceManager::imageLoad(const std::string& path)
    {
        if (!textureImages.contains(path))
        {
            textureImages[path] = LoadImage(path.c_str());
        }
        return textureImages[path];
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
                memcpy(
                    model.materials[i].maps, oldModel.materials[i].maps, MAX_MATERIAL_MAPS * sizeof(MaterialMap));

                // Copy params
                memcpy(model.materials[i].params, oldModel.materials[i].params, 4 * sizeof(float));
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
        if ((model.meshCount != 0) && (model.meshes != NULL))
        {
            // Upload vertex data to GPU (static meshes)
            for (int i = 0; i < model.meshCount; i++)
                UploadMesh(&model.meshes[i], false);
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

        // Copy animation vertex data
        if (oldMesh.animVertices)
        {
            mesh.animVertices = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
            memcpy(mesh.animVertices, oldMesh.animVertices, mesh.vertexCount * 3 * sizeof(float));
        }
        if (oldMesh.animNormals)
        {
            mesh.animNormals = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
            memcpy(mesh.animNormals, oldMesh.animNormals, mesh.vertexCount * 3 * sizeof(float));
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

        mesh.vaoId = 0; // Default value (ensures it gets uploaded to gpu)
    }

    /**
     * Frees model data but keeps the meshes in memory.
     * Allows us to "unpack" a parent model into multiple models from its children.
     * Used for taking in single OBJ files as maps and instantiating the meshes as different entities
     * @param model
     */
    void ResourceManager::UnloadModelKeepMeshes(Model& model)
    {
        // Drop tables (does not unload materials
        for (int i = 0; i < model.materialCount; i++)
            RL_FREE(model.materials[i].maps);

        // Unload arrays
        RL_FREE(model.meshes);
        RL_FREE(model.materials);
        RL_FREE(model.meshMaterial);

        // Unload animation data
        RL_FREE(model.bones);
        RL_FREE(model.bindPose);

        TRACELOG(LOG_INFO, "MODEL: Unloaded model (NOT meshes) from RAM");
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

        if (vsFileName != nullptr)
        {
            if (!vertShaderFileText.contains(vsFileName))
            {
                vertShaderFileText[vsFileName] = LoadFileText(vsFileName);
            }
            vShaderStr = vertShaderFileText[vsFileName];
        }

        if (fsFileName != nullptr)
        {
            if (!fragShaderFileText.contains(fsFileName))
            {
                fragShaderFileText[fsFileName] = LoadFileText(fsFileName);
            }
            fShaderStr = fragShaderFileText[fsFileName];
        }

        return gpuShaderLoad(vShaderStr, fShaderStr);
    }

    Texture ResourceManager::TextureLoad(const std::string& path)
    {
        if (!textures.contains(path))
        {
            textures[path] = LoadTextureFromImage(imageLoad(path));
        }
        return textures[path];
    }

    void ResourceManager::EmplaceModel(const std::string& path)
    {
        EmplaceModel(path, path, path);
    }

    void ResourceManager::EmplaceModel(
        const std::string& modelKey, const std::string& materialKey, const std::string& path)
    {
        if (!modelCopies.contains(modelKey))
        {
            ModelResource modelResource;
            modelResource.model = LoadModel(path.c_str());
            modelResource.materialKey = materialKey;
            modelCopies.try_emplace(modelKey, modelResource);
            if (!modelMaterials.contains(materialKey))
            {
                std::vector<Material> materials(
                    modelResource.model.materials,
                    modelResource.model.materials + modelResource.model.materialCount);
                modelMaterials.try_emplace(materialKey, materials);
            }
        }
    }

    /**
     * @brief Returns a shallow copy of the loaded model
     * NB: Caller should not free the memory.
     * @param path
     * @return Model
     */
    ModelSafe ResourceManager::LoadModelCopy(const std::string& key)
    {
        assert(modelCopies.contains(key));
        ModelSafe modelsafe(modelCopies.at(key).model, true);
        modelsafe.SetKey(key);
        return std::move(modelsafe);
    }

    /**
     * @brief Creates a deep copy of the loaded model. Cuts down model loading times as
     * it's faster copying buffers rather than reading/parsing model files.
     * @param path
     * @return
     */
    ModelSafe ResourceManager::LoadModelDeepCopy(const std::string& key) const
    {
        assert(modelCopies.contains(key));
        Model model;
        deepCopyModel(modelCopies.at(key).model, model);
        return ModelSafe(model);
    }

    ModelAnimation* ResourceManager::ModelAnimationLoad(const std::string& path, int* animsCount)
    {
        if (!modelAnimations.contains(path))
        {
            auto animations = LoadModelAnimations(path.c_str(), animsCount);
            modelAnimations[path] = std::make_pair(animations, *animsCount);
            return animations;
        }
        const auto& pair = modelAnimations[path];
        *animsCount = pair.second;
        return pair.first;
    }

    void ResourceManager::UnloadImages()
    {
        for (const auto& kv : textureImages)
        {
            UnloadImage(kv.second);
        }
        textureImages.clear();
    }

    void ResourceManager::UnloadShaderFileText()
    {
        for (const auto& kv : vertShaderFileText)
        {
            UnloadFileText(kv.second);
        }
        for (const auto& kv : fragShaderFileText)
        {
            UnloadFileText(kv.second);
        }
        vertShaderFileText.clear();
        fragShaderFileText.clear();
    }

    void sgUnloadModel(Model model)
    {
        // Unload meshes
        for (int i = 0; i < model.meshCount; i++)
            UnloadMesh(model.meshes[i]);

        // for (int i = 0; i < model.materialCount; i++)
        //     RL_FREE(model.materials[i].maps);

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
        for (auto& [key, materials] : modelMaterials)
        {
            for (const auto& mat : materials)
            {
                UnloadMaterial(mat);
            }
        }
        for (auto& [path, model] : modelCopies)
        {
            sgUnloadModel(model.model);
        }
        for (const auto& [key, tex] : textures)
        {
            UnloadTexture(tex);
        }
        for (const auto& [key, image] : textureImages)
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
        textures.clear();
        textureImages.clear();
        modelAnimations.clear();
        shaders.clear();
        vertShaderFileText.clear();
        fragShaderFileText.clear();
    }

    ResourceManager::~ResourceManager()
    {
        UnloadAll();
    }

    ResourceManager::ResourceManager()
    {
        Shader shader;
        shader.id = rlGetShaderIdDefault();
        shader.locs = rlGetShaderLocsDefault();
        shaders.emplace("DEFAULT", shader);
    }
} // namespace sage