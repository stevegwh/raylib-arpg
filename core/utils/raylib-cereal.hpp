#pragma once

#include <array>

#include "cereal/archives/binary.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "raylib.h"
#include "raylib/src/config.h"
#include "raymath.h"
#include "rlgl.h"
#include "utils.h"

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

    // Animations
    std::vector<float> animVertices;
    if (mesh.animVertices)
    {
        animVertices.assign(mesh.animVertices, mesh.animVertices + mesh.vertexCount * 3);
    }
    std::vector<float> animNormals;
    if (mesh.animNormals)
    {
        animNormals.assign(mesh.animNormals, mesh.animNormals + mesh.vertexCount * 3);
    }
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
        vertices,
        texcoords,
        texcoords2,
        normals,
        tangents,
        colors,
        animVertices,
        animNormals,
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
    std::vector<float> animVertices;
    std::vector<float> animNormals;
    std::vector<unsigned char> boneIds;
    std::vector<float> boneWeights;

    archive(
        mesh.vertexCount,
        mesh.triangleCount,
        vertices,
        texcoords,
        texcoords2,
        normals,
        tangents,
        colors,
        animVertices,
        animNormals,
        boneIds,
        boneWeights);

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

    // Animations
    if (!animVertices.empty())
    {
        mesh.animVertices = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
        std::memcpy(mesh.animVertices, vertices.data(), mesh.vertexCount * 3 * sizeof(float));
    }
    if (!animNormals.empty())
    {
        mesh.animNormals = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
        std::memcpy(mesh.animNormals, vertices.data(), mesh.vertexCount * 3 * sizeof(float));
    }
    if (!boneIds.empty())
    {
        mesh.boneIds = static_cast<unsigned char*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char)));
        std::memcpy(mesh.boneIds, boneIds.data(), mesh.vertexCount * 4 * sizeof(unsigned char));
    }
    if (!boneWeights.empty())
    {
        mesh.boneWeights = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(float)));
        std::memcpy(mesh.boneWeights, boneWeights.data(), mesh.vertexCount * 4 * sizeof(float));
    }
};

template <typename Archive>
void save(Archive& archive, Image const& image)
{

    unsigned char* _data = static_cast<unsigned char*>(image.data);
    int len = image.format * image.width * image.height;
    std::vector<unsigned char> data(_data, _data + len);
    archive(data, image.format, image.height, image.width, image.mipmaps);
}

template <typename Archive>
void load(Archive& archive, Image& image)
{
    std::vector<unsigned char> data;
    archive(data, image.format, image.height, image.width, image.mipmaps);
    int len = data.size();
    image.data = static_cast<unsigned char*>(RL_MALLOC(len * sizeof(unsigned char)));
    if (image.data != nullptr)
    {
        std::memcpy(image.data, data.data(), len * sizeof(unsigned char));
    }
    else
    {
        // Handle memory allocation failure
    }
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
    image = LoadImageFromTexture(map.texture);
    archive(image, map.color, map.value);
    UnloadImage(image);
};

template <typename Archive>
void load(Archive& archive, MaterialMap& map)
{
    Image image;
    archive(image, map.color, map.value);
    if (!image.data)
    {
        UnloadImage(image);
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
    // model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
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