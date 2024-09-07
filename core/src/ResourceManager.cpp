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
    ResourceManager::ResourceManager()
    {
        Shader shader;
        shader.id = rlGetShaderIdDefault();
        shader.locs = rlGetShaderLocsDefault();
        shaders.emplace("DEFAULT", shader);
    }

    void ResourceManager::deepCopyModel(const Model& oldModel, Model& model)
    {
        model.meshCount = oldModel.meshCount;
        model.materialCount = oldModel.materialCount;
        model.boneCount = oldModel.boneCount;
        model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));
        model.bones = (BoneInfo*)RL_MALLOC(model.boneCount * sizeof(BoneInfo));
        model.bindPose = (Transform*)RL_MALLOC(model.boneCount * sizeof(Transform));

        for (size_t i = 0; i < model.meshCount; ++i)
        {
            deepCopyMesh(oldModel.meshes[i], model.meshes[i]);
        }

        if (model.materialCount == 0)
        {
            // TRACELOG(LOG_WARNING, "MATERIAL: [%s] Failed to load model material data,
            // default to white material", fileName);

            model.materialCount = 1;
            model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
            model.materials[0] = LoadMaterialDefault();

            if (model.meshMaterial == nullptr) model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));
        }
        else
        {
            model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
            model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));

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
                model.materials[i].maps = (MaterialMap*)RL_MALLOC(MAX_MATERIAL_MAPS * sizeof(MaterialMap));
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
        mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
        memcpy(mesh.vertices, oldMesh.vertices, mesh.vertexCount * 3 * sizeof(float));

        if (oldMesh.texcoords)
        {
            mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
            memcpy(mesh.texcoords, oldMesh.texcoords, mesh.vertexCount * 2 * sizeof(float));
        }
        if (oldMesh.texcoords2)
        {
            mesh.texcoords2 = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
            memcpy(mesh.texcoords2, oldMesh.texcoords2, mesh.vertexCount * 2 * sizeof(float));
        }
        if (oldMesh.normals)
        {
            mesh.normals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
            memcpy(mesh.normals, oldMesh.normals, mesh.vertexCount * 3 * sizeof(float));
        }
        if (oldMesh.tangents)
        {
            mesh.tangents = (float*)RL_MALLOC(mesh.vertexCount * 4 * sizeof(float));
            memcpy(mesh.tangents, oldMesh.tangents, mesh.vertexCount * 4 * sizeof(float));
        }
        if (oldMesh.colors)
        {
            mesh.colors = (unsigned char*)RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char));
            memcpy(mesh.colors, oldMesh.colors, mesh.vertexCount * 4 * sizeof(unsigned char));
        }
        if (oldMesh.indices)
        {
            mesh.indices = (unsigned short*)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));
            memcpy(mesh.indices, oldMesh.indices, mesh.triangleCount * 3 * sizeof(unsigned short));
        }

        // Copy animation vertex data
        if (oldMesh.animVertices)
        {
            mesh.animVertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
            memcpy(mesh.animVertices, oldMesh.animVertices, mesh.vertexCount * 3 * sizeof(float));
        }
        if (oldMesh.animNormals)
        {
            mesh.animNormals = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
            memcpy(mesh.animNormals, oldMesh.animNormals, mesh.vertexCount * 3 * sizeof(float));
        }
        if (oldMesh.boneIds)
        {
            mesh.boneIds = (unsigned char*)RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char));
            memcpy(mesh.boneIds, oldMesh.boneIds, mesh.vertexCount * 4 * sizeof(unsigned char));
        }
        if (oldMesh.boneWeights)
        {
            mesh.boneWeights = (float*)RL_MALLOC(mesh.vertexCount * 4 * sizeof(float));
            memcpy(mesh.boneWeights, oldMesh.boneWeights, mesh.vertexCount * 4 * sizeof(float));
        }

        mesh.vaoId = 0; // Default value (ensures it gets uploaded to gpu)

        // Copy name if it exists
        if (oldMesh.name)
        {
            mesh.name = strdup(oldMesh.name);
        }
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

    Image ResourceManager::imageLoad(const std::string& path)
    {
        if (!textureImages.contains(path))
        {
            textureImages[path] = LoadImage(path.c_str());
        }
        return textureImages[path];
    }

    std::vector<entt::entity> ResourceManager::UnpackOBJMap(
        entt::registry* registry, MaterialPaths matPaths, const std::string& mapPath)
    {
        std::vector<entt::entity> out;

        Model parent = LoadModel(mapPath.c_str());
        Matrix modelTransform = MatrixScale(5.0f, 5.0f, 5.0f);
        for (int i = 0; i < parent.meshCount; ++i)
        {
            entt::entity id = registry->create();
            out.push_back(id);

            Model model = LoadModelFromMesh(parent.meshes[i]);

            if (FileExists(matPaths.diffuse.c_str()))
            {
                model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
                    ResourceManager::GetInstance().TextureLoad(matPaths.diffuse);
            }
            if (FileExists(matPaths.specular.c_str()))
            {
                model.materials[0].maps[MATERIAL_MAP_SPECULAR].texture =
                    ResourceManager::GetInstance().TextureLoad(matPaths.specular);
            }
            if (FileExists(matPaths.normal.c_str()))
            {
                model.materials[0].maps[MATERIAL_MAP_NORMAL].texture =
                    ResourceManager::GetInstance().TextureLoad(matPaths.normal);
            }

            auto& renderable = registry->emplace<Renderable>(id, model, matPaths, modelTransform);
            renderable.name = parent.meshes[i].name;
        }
        UnloadModelKeepMeshes(parent);
        return out;
    }

    void ResourceManager::EmplaceModel(const std::string& path)
    {
        if (modelCopies.find(path) == modelCopies.end())
        {
            modelCopies.try_emplace(path, std::make_unique<ModelSafe>(path.c_str()));
        }
    }

    /**
     * @brief Returns a shallow copy of the loaded model
     * NB: Caller should not free the memory.
     * @param path
     * @return Model
     */
    std::unique_ptr<ModelSafe> ResourceManager::LoadModelCopy(const std::string& path)
    {
        EmplaceModel(path);
        Model model;
        model = modelCopies.at(path)->rlmodel;
        return std::make_unique<ModelSafe>(model, true); // TODO: pointless this being a shared_ptr
    }

    /**
     * @brief Creates a deep copy of the loaded model. Cuts down model loading times as
     * it's faster copying buffers rather than reading/parsing model files.
     * @param path
     * @return
     */
    std::unique_ptr<ModelSafe> ResourceManager::LoadModelDeepCopy(const std::string& path)
    {
        Model model;
        EmplaceModel(path);
        deepCopyModel(modelCopies.at(path)->rlmodel, model);
        return std::make_unique<ModelSafe>(model);
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

    void ResourceManager::UnloadAll()
    {
        for (auto& [path, modelSafe] : modelCopies)
        {
            modelSafe.reset();
        }
        for (const auto& kv : textures)
        {
            UnloadTexture(kv.second);
        }
        for (const auto& kv : textureImages)
        {
            UnloadImage(kv.second);
        }
        for (const auto& kv : modelAnimations)
        {
            UnloadModelAnimations(kv.second.first, kv.second.second);
        }
        for (const auto& kv : shaders)
        {
            UnloadShader(kv.second);
        }
        for (const auto& kv : vertShaderFileText)
        {
            UnloadFileText(kv.second);
        }
        for (const auto& kv : fragShaderFileText)
        {
            UnloadFileText(kv.second);
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
} // namespace sage