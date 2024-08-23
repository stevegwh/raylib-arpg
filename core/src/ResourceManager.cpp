//
// Created by Steve Wheeler on 16/07/2024.
//

#include "ResourceManager.hpp"

#include <string.h>
#include <unordered_map>
#include <utility>

#include "raylib/src/config.h"
#include "raymath.h"
#include "rlgl.h"

namespace sage
{

    static std::unordered_map<std::string, Image> textureImages;
    static std::unordered_map<std::string, Model> staticModels;
    static std::unordered_map<std::string, Model> dynamicModels;
    static std::unordered_map<std::string, std::pair<ModelAnimation*, int>> modelAnimations;
    static std::unordered_map<std::string, char*> vertShaders;
    static std::unordered_map<std::string, char*> fragShaders;

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
        Shader shader = {0};

        char* vShaderStr = nullptr;
        char* fShaderStr = nullptr;

        if (vsFileName != nullptr)
        {
            if (vertShaders.find(vsFileName) == vertShaders.end())
            {
                vertShaders[vsFileName] = LoadFileText(vsFileName);
            }
            vShaderStr = vertShaders[vsFileName];
        }

        if (fsFileName != nullptr)
        {
            if (fragShaders.find(fsFileName) == fragShaders.end())
            {
                fragShaders[fsFileName] = LoadFileText(fsFileName);
            }
            fShaderStr = fragShaders[fsFileName];
        }

        shader = LoadShaderFromMemory(vShaderStr, fShaderStr);

        return shader;
    }

    Image ResourceManager::ImageLoad(const std::string& path)
    {
        if (textureImages.find(path) == textureImages.end())
        {
            Image img = LoadImage(path.c_str());
            textureImages[path] = img;
            return img;
        }
        return textureImages[path];
    }

    /**
     * @brief Returns a shallow copy of the loaded model
     *
     * @param path
     * @return Model
     */
    Model ResourceManager::StaticModelLoad(const std::string& path)
    {
        if (staticModels.find(path) == staticModels.end())
        {
            Model model = LoadModel(path.c_str());
            staticModels[path] = model;
            return model;
        }

        return staticModels[path];
    }

    Mesh deepCopyMesh(const Mesh& oldMesh, Mesh& mesh)
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

        return mesh;
    }

    /**
     * @brief Creates a deep copy of the loaded model. Cuts down model loading times as
     * it's faster copying buffers rather than reading/parsing model files.
     * @param path
     * @return
     */
    Model ResourceManager::DynamicModelLoad(const std::string& path)
    {
        if (dynamicModels.find(path) == dynamicModels.end())
        {
            Model model = LoadModel(path.c_str());
            dynamicModels[path] = model;
        }
        Model model;
        const Model& oldModel = dynamicModels[path];
        // deep copy model here
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

            if (model.meshMaterial == NULL) model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));
        }
        else
        {
            model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
            model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));

            for (size_t i = 0; i < model.materialCount; ++i)
            {
                model.materials[i] = oldModel.materials[i];

                // Deep copy shader
                model.materials[i].shader = oldModel.materials[i].shader;
                model.materials[i].shader.locs = (int*)RL_MALLOC(RL_MAX_SHADER_LOCATIONS * sizeof(int));
                memcpy(
                    model.materials[i].shader.locs,
                    oldModel.materials[i].shader.locs,
                    RL_MAX_SHADER_LOCATIONS * sizeof(int));

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

        return model;
    }

    ModelAnimation* ResourceManager::ModelAnimationLoad(const std::string& path, int* animsCount)
    {
        if (modelAnimations.find(path) == modelAnimations.end())
        {
            auto animations = LoadModelAnimations(path.c_str(), animsCount);
            modelAnimations[path] = std::make_pair(animations, *animsCount);
            return animations;
        }
        const auto& pair = modelAnimations[path];
        *animsCount = pair.second;
        return pair.first;
    }

    ResourceManager::~ResourceManager()
    {
        for (auto kv : textureImages)
        {
            UnloadImage(kv.second);
        }
        for (auto kv : staticModels)
        {
            UnloadModel(kv.second);
        }
        for (auto kv : dynamicModels)
        {
            UnloadModel(kv.second);
        }
        for (auto kv : modelAnimations)
        {
            UnloadModelAnimations(kv.second.first, kv.second.second);
        }
        for (auto kv : vertShaders)
        {
            UnloadFileText(kv.second);
        }
        for (auto kv : fragShaders)
        {
            UnloadFileText(kv.second);
        }
    }
} // namespace sage