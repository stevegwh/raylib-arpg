#pragma once

#include "AssetID.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "magic_enum.hpp"
#include "raylib.h"
#include "raylib/src/config.h"
#include "raymath.h"
#include "rlgl.h"
#include <array>
#include <cstring>

namespace cereal
{
    template <class Archive>
    inline std::string save_minimal(Archive const&, const sage::AssetID& t)
    {
        return std::string(magic_enum::enum_name(t));
    }

    template <class Archive>
    inline void load_minimal(Archive const&, sage::AssetID& t, std::string const& value)
    {
        t = magic_enum::enum_cast<sage::AssetID>(value).value();
    }
} // namespace cereal

template <typename Archive>
void serialize(Archive& archive, Vector2& v2)
{
    archive(v2.x, v2.y);
};

template <typename Archive>
void serialize(Archive& archive, Vector3& v3)
{
    archive(v3.x, v3.y, v3.z);
};

template <typename Archive>
void serialize(Archive& archive, Vector4& v4)
{
    archive(v4.x, v4.y, v4.z, v4.w);
};

template <typename Archive>
void serialize(Archive& archive, Transform& transform)
{
    archive(transform.translation, transform.rotation, transform.scale);
};

template <typename Archive>
void serialize(Archive& archive, Matrix& m)
{
    archive(m.m0, m.m1, m.m2, m.m3, m.m4, m.m5, m.m6, m.m7, m.m8, m.m9, m.m10, m.m11, m.m12, m.m13, m.m14, m.m15);
};

template <typename Archive>
void serialize(Archive& archive, BoundingBox& bb)
{
    archive(bb.min, bb.max);
};

template <typename Archive>
void save(Archive& archive, ModelAnimation const& modelAnimation)
{
    std::vector<BoneInfo> bones(modelAnimation.bones, modelAnimation.bones + modelAnimation.boneCount);
    std::vector<std::vector<Transform>> framePoses(modelAnimation.frameCount);

    for (int i = 0; i < modelAnimation.frameCount; i++)
    {
        framePoses[i].assign(
            modelAnimation.framePoses[i], modelAnimation.framePoses[i] + modelAnimation.boneCount);
    }

    archive(modelAnimation.boneCount, modelAnimation.frameCount, bones, framePoses, modelAnimation.name);
}

template <typename Archive>
void load(Archive& archive, ModelAnimation& modelAnimation)
{
    std::vector<BoneInfo> bones;
    std::vector<std::vector<Transform>> framePoses;

    archive(modelAnimation.boneCount, modelAnimation.frameCount, bones, framePoses, modelAnimation.name);

    modelAnimation.bones = static_cast<BoneInfo*>(RL_MALLOC(modelAnimation.boneCount * sizeof(BoneInfo)));
    std::copy(bones.begin(), bones.end(), modelAnimation.bones);

    modelAnimation.framePoses =
        static_cast<Transform**>(RL_MALLOC(modelAnimation.frameCount * sizeof(Transform*)));
    for (int i = 0; i < modelAnimation.frameCount; i++)
    {
        modelAnimation.framePoses[i] =
            static_cast<Transform*>(RL_MALLOC(modelAnimation.boneCount * sizeof(Transform)));
        std::copy(framePoses[i].begin(), framePoses[i].end(), modelAnimation.framePoses[i]);
    }
}

template <typename Archive>
void save(Archive& archive, Mesh const& mesh)
{
    std::vector<float> vertices(mesh.vertices, mesh.vertices + mesh.vertexCount * 3); // vec3

    std::vector<float> texcoords;
    if (mesh.texcoords)
    {
        texcoords.assign(mesh.texcoords, mesh.texcoords + mesh.vertexCount * 2); // vec2
    }

    std::vector<float> texcoords2;
    if (mesh.texcoords2)
    {
        texcoords2.assign(mesh.texcoords2, mesh.texcoords2 + mesh.vertexCount * 2); // vec2
    }

    std::vector<float> normals;
    if (mesh.normals)
    {
        normals.assign(mesh.normals, mesh.normals + mesh.vertexCount * 3); // vec3
    }

    std::vector<float> tangents;
    if (mesh.tangents)
    {
        tangents.assign(mesh.tangents, mesh.tangents + mesh.vertexCount * 4); // vec4
    }

    std::vector<unsigned char> colors;
    if (mesh.colors)
    {
        colors.assign(mesh.colors, mesh.colors + mesh.vertexCount * 4); // vec4
    }

    std::vector<unsigned short> indices;
    if (mesh.indices)
    {
        indices.assign(mesh.indices, mesh.indices + mesh.triangleCount * 3);
    }

    // Animations
    std::vector<unsigned char> boneIds;
    if (mesh.boneIds)
    {
        boneIds.assign(mesh.boneIds, mesh.boneIds + mesh.vertexCount * 4);
    }
    std::vector<float> boneWeights;
    if (mesh.boneWeights)
    {
        boneWeights.assign(mesh.boneWeights, mesh.boneWeights + mesh.vertexCount * 4); // vec4
    }

    archive(
        mesh.vertexCount,
        mesh.triangleCount,
        mesh.boneCount,
        vertices,
        texcoords,
        texcoords2,
        normals,
        tangents,
        colors,
        indices,
        boneIds,
        boneWeights);
}

template <typename Archive>
void load(Archive& archive, Mesh& mesh)
{
    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<float> texcoords2;
    std::vector<float> normals;
    std::vector<float> tangents;
    std::vector<unsigned char> colors;
    std::vector<unsigned short> indices;
    std::vector<unsigned char> boneIds;
    std::vector<float> boneWeights;

    archive(
        mesh.vertexCount,
        mesh.triangleCount,
        mesh.boneCount,
        vertices,
        texcoords,
        texcoords2,
        normals,
        tangents,
        colors,
        indices,
        boneIds,
        boneWeights);

    bool animations = !boneIds.empty();

    mesh.vertices = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
    std::memcpy(mesh.vertices, vertices.data(), mesh.vertexCount * 3 * sizeof(float));

    if (!texcoords.empty())
    {
        mesh.texcoords = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 2 * sizeof(float)));
        std::memcpy(mesh.texcoords, texcoords.data(), mesh.vertexCount * 2 * sizeof(float));
    }
    if (!texcoords2.empty())
    {
        mesh.texcoords2 = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 2 * sizeof(float)));
        std::memcpy(mesh.texcoords2, texcoords2.data(), mesh.vertexCount * 2 * sizeof(float));
    }
    if (!normals.empty())
    {
        mesh.normals = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
        std::memcpy(mesh.normals, normals.data(), mesh.vertexCount * 3 * sizeof(float));
    }
    if (!tangents.empty())
    {
        mesh.tangents = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(float)));
        std::memcpy(mesh.tangents, tangents.data(), mesh.vertexCount * 4 * sizeof(float));
    }
    if (!colors.empty())
    {
        mesh.colors = static_cast<unsigned char*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char)));
        std::memcpy(mesh.colors, colors.data(), mesh.vertexCount * 4 * sizeof(unsigned char));
    }
    if (!indices.empty())
    {
        mesh.indices = static_cast<unsigned short*>(RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short)));
        std::memcpy(mesh.indices, indices.data(), mesh.triangleCount * 3 * sizeof(unsigned short));
    }

    // Animations
    if (animations)
    {
        mesh.animVertices = static_cast<float*>(RL_CALLOC(mesh.vertexCount * 3, sizeof(float)));
        std::memcpy(mesh.animVertices, vertices.data(), mesh.vertexCount * 3 * sizeof(float));
        mesh.animNormals = static_cast<float*>(RL_CALLOC(mesh.vertexCount * 3, sizeof(float)));
        std::memcpy(mesh.animNormals, normals.data(), mesh.vertexCount * 3 * sizeof(float));

        mesh.boneIds = static_cast<unsigned char*>(RL_CALLOC(mesh.vertexCount * 4, sizeof(unsigned char)));
        std::memcpy(mesh.boneIds, boneIds.data(), mesh.vertexCount * 4 * sizeof(unsigned char));

        mesh.boneWeights = static_cast<float*>(RL_CALLOC(mesh.vertexCount * 4, sizeof(float)));
        std::memcpy(mesh.boneWeights, boneWeights.data(), mesh.vertexCount * 4 * sizeof(float));

        mesh.boneMatrices = static_cast<Matrix*>(RL_CALLOC(mesh.boneCount, sizeof(Matrix)));
        for (int j = 0; j < mesh.boneCount; j++)
        {
            mesh.boneMatrices[j] = MatrixIdentity();
            // Gets updated per animation, no need to copy info over.
        }
    }
};

template <typename Archive>
void save(Archive& archive, Image const& image)
{
    unsigned char* rlData = static_cast<unsigned char*>(image.data);
    int len = GetPixelDataSize(image.width, image.height, image.format);
    std::vector<unsigned char> data(rlData, rlData + len);
    archive(data, image.format, image.height, image.width, image.mipmaps);
}

template <typename Archive>
void load(Archive& archive, Image& image)
{
    std::vector<unsigned char> data;
    archive(data, image.format, image.height, image.width, image.mipmaps);
    int len = GetPixelDataSize(image.width, image.height, image.format);
    image.data = static_cast<unsigned char*>(RL_MALLOC(len * sizeof(unsigned char)));
    std::memcpy(image.data, data.data(), len * sizeof(unsigned char));
}

template <typename Archive>
void save(Archive& archive, Shader const& shader)
{
    std::vector<int> locs(shader.locs, shader.locs + RL_MAX_SHADER_LOCATIONS);
    archive(shader.id, locs);
};

template <typename Archive>
void load(Archive& archive, Shader& shader)
{
    std::vector<int> locs;
    archive(shader.id, locs);
    shader.locs = static_cast<int*>(RL_MALLOC(RL_MAX_SHADER_LOCATIONS * sizeof(int)));
    std::memcpy(shader.locs, locs.data(), RL_MAX_SHADER_LOCATIONS * sizeof(int));
};

template <typename Archive>
void serialize(Archive& archive, Color& color)
{
    archive(color.r, color.g, color.b, color.a);
};

template <typename Archive>
void save(Archive& archive, MaterialMap const& map)
{
    Image image{};
    image.format = map.texture.format;

    if (map.texture.format < PIXELFORMAT_COMPRESSED_DXT1_RGB && map.texture.id != rlGetTextureIdDefault() &&
        map.texture.id != 0)
    {
        image = LoadImageFromTexture(map.texture);
    }

    archive(image, map.color, map.value);
    UnloadImage(image);
};

template <typename Archive>
void load(Archive& archive, MaterialMap& map)
{
    Image image;
    archive(image, map.color, map.value);
    if (!image.data || (image.width == 0 && image.height == 0) || image.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        map.texture = Texture2D{rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};

        if (image.data)
        {
            UnloadImage(image);
        }

        return;
    }
    map.texture = LoadTextureFromImage(image);
    UnloadImage(image);
};

template <typename Archive>
void save(Archive& archive, Material const& material)
{
    std::vector<MaterialMap> maps;
    maps.resize(MAX_MATERIAL_MAPS);
    std::array<float, 4> params{};

    for (size_t i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (maps[i].texture.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB ||
            maps[i].texture.id == rlGetTextureIdDefault())
            continue;
        maps[i] = material.maps[i];
    }
    for (size_t i = 0; i < 4; i++)
    {
        params[i] = material.params[i];
    }
    archive(maps, params);
};

template <typename Archive>
void load(Archive& archive, Material& material)
{
    std::vector<MaterialMap> maps;
    maps.resize(MAX_MATERIAL_MAPS);
    std::array<float, 4> params{};

    archive(maps, params);

    material = LoadMaterialDefault();
    //  material.maps[MATERIAL_MAP_DIFFUSE] = maps.at(0);
    std::memcpy(material.maps, maps.data(), MAX_MATERIAL_MAPS * sizeof(MaterialMap));
};

template <typename Archive>
void serialize(Archive& archive, BoneInfo& boneInfo)
{
    archive(boneInfo.name, boneInfo.parent);
};

template <typename Archive>
void save(Archive& archive, Model const& model)
{
    std::vector<Mesh> meshes(model.meshes, model.meshes + model.meshCount);
    // std::vector<Material> materials(model.materials, model.materials+model.materialCount);
    std::vector<BoneInfo> bones(model.bones, model.bones + model.boneCount);
    std::vector<Transform> bindPose(model.bindPose, model.bindPose + model.boneCount);
    std::vector<int> meshMaterial(model.meshMaterial, model.meshMaterial + model.meshCount);

    archive(
        model.transform,
        model.meshCount,
        model.materialCount,
        meshes,
        // materials,
        meshMaterial,
        model.boneCount,
        bones,
        bindPose);
};

template <typename Archive>
void load(Archive& archive, Model& model)
{
    std::vector<Mesh> meshes;
    // std::vector<Material> materials;
    std::vector<int> meshMaterial;
    std::vector<BoneInfo> bones;
    std::vector<Transform> bindPose;

    archive(
        model.transform,
        model.meshCount,
        model.materialCount,
        meshes,
        // materials,
        meshMaterial,
        model.boneCount,
        bones,
        bindPose);

    model.meshes = static_cast<Mesh*>(RL_CALLOC(model.meshCount, sizeof(Mesh)));
    model.materials = static_cast<Material*>(RL_CALLOC(model.materialCount, sizeof(Material)));

    for (unsigned int i = 0; i < model.materialCount; ++i)
    {
        model.materials[i] = LoadMaterialDefault();
    }

    model.meshMaterial = static_cast<int*>(RL_CALLOC(model.meshCount, sizeof(int)));
    model.bones = static_cast<BoneInfo*>(RL_MALLOC(model.boneCount * sizeof(BoneInfo)));
    model.bindPose = static_cast<Transform*>(RL_MALLOC(model.boneCount * sizeof(Transform)));

    std::memcpy(model.meshes, meshes.data(), model.meshCount * sizeof(Mesh));
    // std::memcpy(model.materials, materials.data(), model.materialCount * sizeof(Material);
    std::memcpy(model.meshMaterial, meshMaterial.data(), model.meshCount * sizeof(int));
    std::memcpy(model.bones, bones.data(), model.boneCount * sizeof(BoneInfo));
    std::memcpy(model.bindPose, bindPose.data(), model.boneCount * sizeof(Transform));

    // Below taken from raylib's LoadModel().
    model.transform = MatrixIdentity();
    if ((model.meshCount != 0) && (model.meshes != nullptr))
    {
        // Upload vertex data to GPU (static meshes)
        for (int i = 0; i < model.meshCount; i++)
            UploadMesh(&model.meshes[i], false);
    }
    else
        TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load model mesh(es) data", "Cereal Model Import");
};