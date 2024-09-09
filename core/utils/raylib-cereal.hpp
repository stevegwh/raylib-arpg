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
void save(Archive& archive, Image const& image)
{
    std::vector<unsigned char> data;
    unsigned char* _data = static_cast<unsigned char*>(image.data);
    int len = image.format * image.width * image.height;
    data.reserve(len);
    for (int i = 0; i < len; ++i)
    {
        data.push_back(_data[i]);
    }
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
        std::memcpy(image.data, data.data(), len);
    }
    else
    {
        // Handle memory allocation failure
    }
}

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
    std::vector<Vector3> vertices;
    std::vector<Vector2> texcoords;
    std::vector<Vector2> texcoords2;
    std::vector<Vector3> normals;
    std::vector<Vector4> tangents;
    std::vector<Color> colors;

    // Animations not supported right now
    // std::vector<Vector3> animVertices;
    // std::vector<Vector3> animNormals;
    // std::vector<std::string> boneIds; // Unsigned char*
    // std::vector<Vector4> boneWeights; // float* (4 bones per vertex)
    // TODO: Bone IDs and weights are missing

    vertices.reserve(mesh.vertexCount * 3);
    texcoords.reserve(mesh.vertexCount * 2);
    texcoords2.reserve(mesh.vertexCount * 2);
    normals.reserve(mesh.vertexCount * 3);
    tangents.reserve(mesh.vertexCount * 4);
    colors.reserve(mesh.vertexCount * 4);

    // animVertices.reserve(mesh.vertexCount*3);
    // animNormals.reserve(mesh.vertexCount*3);

    for (int i = 0; i < mesh.vertexCount; ++i)
    {
        vertices.push_back({mesh.vertices[i * 3], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2]});
        if (mesh.texcoords)
        {
            texcoords.push_back({mesh.texcoords[i * 2], mesh.texcoords[i * 2 + 1]});
        }

        if (mesh.texcoords2)
        {
            texcoords2.push_back({mesh.texcoords2[i * 2], mesh.texcoords2[i * 2 + 1]});
        }
        if (mesh.normals)
        {
            normals.push_back({mesh.normals[i * 3], mesh.normals[i * 3 + 1], mesh.normals[i * 3 + 2]});
        }
        if (mesh.tangents)
        {
            tangents.push_back(
                {mesh.tangents[i * 4],
                 mesh.tangents[i * 4 + 1],
                 mesh.tangents[i * 4 + 2],
                 mesh.tangents[i * 4 + 3]});
        }
        if (mesh.colors)
        {
            colors.push_back(
                {mesh.colors[i * 4], mesh.colors[i * 4 + 1], mesh.colors[i * 4 + 2], mesh.colors[i * 4 + 3]});
        }
        // if (animVertices)
        //{
        //	animVertices.push_back({
        //		mesh.animVertices[i * 3],
        //		mesh.animVertices[i * 3 + 1],
        //		mesh.animVertices[i * 3 + 2]
        //	});
        // }
        // if (animNormals)
        //{
        //	animNormals.push_back({
        //		mesh.animNormals[i * 3],
        //		mesh.animNormals[i * 3 + 1],
        //		mesh.animNormals[i * 3 + 2]
        //	});
        // }
    }
    archive(
        mesh.vertexCount, mesh.triangleCount, vertices, texcoords, texcoords2, normals, tangents, colors
        // animVertices,
        // animNormals,
    );
};

template <typename Archive>
void load(Archive& archive, Mesh& mesh)
{
    std::vector<Vector3> vertices;
    std::vector<Vector2> texcoords;
    std::vector<Vector2> texcoords2;
    std::vector<Vector3> normals;
    std::vector<Vector4> tangents;
    std::vector<Color> colors;

    archive(mesh.vertexCount, mesh.triangleCount, vertices, texcoords, texcoords2, normals, tangents, colors);

    mesh.vertices = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
    if (!texcoords.empty()) mesh.texcoords = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 2 * sizeof(float)));
    if (!texcoords2.empty())
        mesh.texcoords2 = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 2 * sizeof(float)));
    if (!normals.empty()) mesh.normals = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 3 * sizeof(float)));
    if (!tangents.empty()) mesh.tangents = static_cast<float*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(float)));
    if (!colors.empty())
        mesh.colors = static_cast<unsigned char*>(RL_MALLOC(mesh.vertexCount * 4 * sizeof(unsigned char)));

    for (int i = 0; i < vertices.size(); ++i)
    {
        mesh.vertices[i * 3] = vertices[i].x;
        mesh.vertices[i * 3 + 1] = vertices[i].y;
        mesh.vertices[i * 3 + 2] = vertices[i].z;
        if (!texcoords.empty())
        {
            mesh.texcoords[i * 2] = texcoords[i].x;
            mesh.texcoords[i * 2 + 1] = texcoords[i].y;
        }
        if (!texcoords2.empty())
        {
            mesh.texcoords2[i * 2] = texcoords2[i].x;
            mesh.texcoords2[i * 2 + 1] = texcoords2[i].y;
        }
        if (!normals.empty())
        {
            mesh.normals[i * 3] = normals[i].x;
            mesh.normals[i * 3 + 1] = normals[i].y;
            mesh.normals[i * 3 + 2] = normals[i].z;
        }
        if (!tangents.empty())
        {
            mesh.tangents[i * 4] = tangents[i].x;
            mesh.tangents[i * 4 + 1] = tangents[i].y;
            mesh.tangents[i * 4 + 2] = tangents[i].z;
            mesh.tangents[i * 4 + 3] = tangents[i].w;
        }
        if (!colors.empty())
        {
            mesh.colors[i * 4] = colors[i].r;
            mesh.colors[i * 4 + 1] = colors[i].g;
            mesh.colors[i * 4 + 2] = colors[i].b;
            mesh.colors[i * 4 + 3] = colors[i].a;
        }
    }
};

template <typename Archive>
void save(Archive& archive, Shader const& shader)
{
    std::vector<int> locs;
    locs.reserve(RL_MAX_SHADER_LOCATIONS);
    for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++)
    {
        locs.push_back(shader.locs[i]);
    }
    archive(shader.id, locs);
};

template <typename Archive>
void load(Archive& archive, Shader& shader)
{
    std::vector<int> locs;
    locs.reserve(RL_MAX_SHADER_LOCATIONS);
    shader.locs = static_cast<int*>(RL_MALLOC(RL_MAX_SHADER_LOCATIONS * sizeof(int)));
    archive(shader.id, locs);
    for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++)
    {
        shader.locs[i] = locs[i];
    }
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
    material = LoadMaterialDefault();
    std::vector<MaterialMap> maps;
    maps.resize(MAX_MATERIAL_MAPS);
    std::array<float, 4> params{};

    archive(maps, params);
    for (size_t i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        material.maps[i] = maps[i];
    }
};

template <typename Archive>
void serialize(Archive& archive, BoneInfo& boneInfo)
{
    archive(boneInfo.name, boneInfo.parent);
};

template <typename Archive>
void save(Archive& archive, Model const& model)
{
    std::vector<Mesh> meshes;
    meshes.reserve(model.meshCount);
    for (size_t i = 0; i < model.meshCount; i++)
    {
        meshes.push_back(model.meshes[i]);
    }

    // std::vector<Material> materials;
    // materials.reserve(model.materialCount);
    // for (size_t i = 0; i < model.materialCount; i++)
    // {
    //     materials.push_back(model.materials[i]);
    // }

    std::vector<BoneInfo> bones;
    bones.reserve(model.boneCount);
    for (size_t i = 0; i < model.boneCount; i++)
    {
        bones.push_back(model.bones[i]);
    }

    std::vector<Transform> bindPose;
    bindPose.reserve(model.boneCount);
    for (size_t i = 0; i < model.boneCount; i++)
    {
        bindPose.push_back(model.bindPose[i]);
    }

    std::vector<int> meshMaterial;
    meshMaterial.reserve(model.boneCount);
    for (size_t i = 0; i < model.meshCount; i++)
    {
        meshMaterial.push_back(model.meshMaterial[i]);
    }

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

    for (size_t i = 0; i < model.meshCount; ++i)
    {
        model.meshes[i] = meshes[i];
    }
    // for (size_t i = 0; i < model.materialCount; ++i)
    // {
    //     model.materials[i] = LoadMaterialDefault(); // TODO: Redundant?
    //     model.materials[i] = materials[i];
    // }
    for (size_t i = 0; i < model.meshCount; ++i)
    {
        model.meshMaterial[i] = meshMaterial[i];
    }
    for (size_t i = 0; i < model.boneCount; ++i)
    {
        model.bones[i] = bones[i];
    }
    for (size_t i = 0; i < model.boneCount; ++i)
    {
        model.bindPose[i] = bindPose[i];
    }

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