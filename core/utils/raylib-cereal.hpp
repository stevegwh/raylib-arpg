#pragma once

#include <array>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"
#include "raylib.h"


template <typename Archive>
void serialize(Archive& archive, Vector3& v3)
{
	archive(v3.x, v3.y, v3.z);
};

template <typename Archive>
void serialize(Archive& archive, Vector2& v2)
{
	archive(v2.x, v2.y);
};

template <typename Archive>
void serialize(Archive& archive, Matrix& m)
{
	archive(
		m.m0,
		m.m1,
		m.m2,
		m.m3,
		m.m4,
		m.m5,
		m.m6,
		m.m7,
		m.m8,
		m.m9,
		m.m10,
		m.m11,
		m.m12,
		m.m13,
		m.m14,
		m.m15
		);
};

template <typename Archive>
void serialize(Archive& archive, BoundingBox& bb)
{
	archive(
		bb.min,
		bb.max
		);
};


//char* name;             // Name of mesh (if applicable)
//int vertexCount;        // Number of vertices stored in arrays
//int triangleCount;      // Number of triangles stored (indexed or not)

//// Vertex attributes data
//float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
//float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
//float *texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
//float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
//float *tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
//unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
//unsigned short *indices;    // Vertex indices (in case vertex data comes indexed)

//// Animation vertex data
//float *animVertices;    // Animated vertex positions (after bones transformations)
//float *animNormals;     // Animated normals (after bones transformations)
//unsigned char *boneIds; // Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning)
//float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning)
template <typename Archive>
void save(Archive& archive, Mesh const & mesh)
{
    std::vector<Vector3> vertices;
    std::vector<Vector2> texcoords;
	std::vector<Vector3> normals;
	std::vector<Vector3> animVertices;
	std::vector<Vector3> animNormals;
    
    vertices.reserve(mesh.vertexCount);
    texcoords.reserve(mesh.vertexCount);
    normals.reserve(mesh.vertexCount);
    
    for (int i = 0; i < mesh.vertexCount; ++i) 
    {
        // Convert vertices
        vertices.push_back({
            mesh.vertices[i * 3],
            mesh.vertices[i * 3 + 1],
            mesh.vertices[i * 3 + 2]
        });
        
        // Convert texcoords
        texcoords.push_back({
            mesh.texcoords[i * 2],
            mesh.texcoords[i * 2 + 1]
        });
        
        // Convert normals
        normals.push_back({
            mesh.normals[i * 3],
            mesh.normals[i * 3 + 1],
            mesh.normals[i * 3 + 2]
        });
    }
	archive(
		mesh.vertexCount,
		mesh.triangleCount,
		vertices,
		texcoords,
        normals,
		// tangents
		animVertices,
		animNormals,
		);
};

template <typename Archive>
void load(Archive& archive, Mesh& mesh)
{
    std::vector<Vector3> vertices;
    std::vector<Vector2> texcoords;
    std::vector<Vector3> normals;

    
    Model _model;
	archive(
		vertices,
		texcoords,
		normals
	);
};

template <typename Archive>
void save(Archive& archive, Shader const &  shader)
{
	std::vector<int> locs;
	locs.reserve(MAX_SHADER_LOCATIONS);
	for (int i = 0; i < MAX_SHADER_LOCATIONS; i++)
	{
		locs.push_back(shader.locs[i]);
	}
	archive (
		shader.id,
		locs
	);
};

template <typename Archive>
void serialize(Archive& archive, Color &  color)
{
	archive(
		color.r,
		color.g,
		color.b,
		color.a
	);
};


template <typename Archive>
void serialize(Archive& archive, Texture2D &  texture)
{
	archive(
		texture.width,
		texture.height,
		texture.mipmaps,
		texture.format
	);
};

template <typename Archive>
void serialize(Archive& archive, Texture &  texture)
{
	archive(
		texture.width,
		texture.height,
		texture.mipmaps,
		texture.format
	);
};

template <typename Archive>
void serialize(Archive& archive, Vector4 &  v4)
{
	archive(
		v4.x,
		v4.y,
		v4.z,
		v4.w
	);
};

template <typename Archive>
void serialize(Archive& archive, Quaternion &  v4)
{
	archive(
		v4.x,
		v4.y,
		v4.z,
		v4.w
	);
};

template <typename Archive>
void serialize(Archive& archive, Transform &  transform)
{
	archive(
		transform.scale,
		transform.translation,
		transform.rotation
	);
};

template <typename Archive>
void serialize(Archive& archive, MaterialMap &  map)
{
	archive(
		map.color,
		map.texture,
		map.value
	);
};

template <typename Archive>
void serialize(Archive& archive, Material &  material)
{
	archive(
		material.shader,
		material.maps,
		material.params
	);
};

template <typename Archive>
void serialize(Archive& archive, BoneInfo &  boneInfo)
{
	archive(
		boneInfo.name,
		boneInfo.parent
	);
};

//Matrix transform;       // Local transform matrix
//int meshCount;          // Number of meshes
//int materialCount;      // Number of materials
//Mesh *meshes;           // Meshes array
//Material *materials;    // Materials array
//int *meshMaterial;      // Mesh material number
//// Animation data
//int boneCount;          // Number of bones
//BoneInfo *bones;        // Bones information (skeleton)
//Transform *bindPose;    // Bones base transformation (pose)
template <typename Archive>
void save(Archive& archive, Model const &  model)
{
    std::vector<Mesh> meshes;
	meshes.reserve(model.meshCount);
	for (size_t i = 0; i < model.meshCount; i++)
	{
		meshes.push_back(model.meshes[i]);
	}

	std::vector<Material> materials;
	materials.reserve(model.materialCount);
	for (size_t i = 0; i < model.materialCount; i++)
	{
		materials.push_back(model.materials[i]);
	}
	
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

	archive(
		model.transform,
		model.meshCount,
		model.materialCount,
		meshes,
		materials,
		model.meshMaterial,
		model.boneCount,
		bones,
		bindPose
	);
};

template <typename Archive>
void load(Archive& archive, Model& model)
{
    std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::vector<BoneInfo> bones;
	std::vector<Transform> bindPose;

	model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));
	model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
	model.bones = RL_MALLOC(model.boneCount*sizeof(BoneInfo));
    model.bindPose = RL_MALLOC(model.boneCount*sizeof(Transform));
    
	archive(
		model.transform,
		model.meshCount,
		model.materialCount,
		meshes,
		materials,
		model.meshMaterial,
		model.boneCount,
		bones,
		bindPose
	);
	
	for (size_t i = 0; i < model.meshCount; ++i)
	{
		model.meshes[i] = meshes[i];
	}
	for (size_t i = 0; i < model.materialCount; ++i)
	{
		model.materials[i] = materials[i];
	}
	for (size_t i = 0; i < model.boneCount; ++i)
	{
		model.bones[i] = bones[i];
	}
	for (size_t i = 0; i < model.boneCount; ++i)
	{
		model.bindPose[i] = bindPose[i];
	}
};