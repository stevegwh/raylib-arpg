//
// Created by Steve Wheeler on 26/12/2024.
//

#include "Slibmodel.hpp"

#include "raylib.h" // Declares module functions

#include <vector>

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" // Defines module configuration flags
#endif

#if defined(SUPPORT_MODULE_RMODELS)

#include "raymath.h" // Required for: Vector3, Quaternion and Matrix functionality
#include "rlgl.h"    // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2
#include "utils.h"   // Required for: TRACELOG(), LoadFileData(), LoadFileText(), SaveFileText()

#include <cassert>
#include <cstdlib> // Required for: malloc(), calloc(), free()
#include <cstring> // Required for: memcmp(), strlen(), strncpy()

#if defined(SUPPORT_FILEFORMAT_OBJ) || defined(SUPPORT_FILEFORMAT_MTL)

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#endif

#if defined(SUPPORT_FILEFORMAT_GLTF)
#define CGLTF_MALLOC RL_MALLOC
#define CGLTF_FREE RL_FREE

#include "external/cgltf.h" // glTF file format loading
#endif

namespace sage
{

    //----------------------------------------------------------------------------------
    // Module specific Functions Declaration
    //----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_OBJ)
    static Model sgLoadOBJ(const char* fileName); // Load OBJ mesh data
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
    static Model sgLoadGLTF(const char* fileName); // Load GLTF mesh data
    static ModelAnimation* sgLoadModelAnimationsGLTF(
        const char* fileName, int* animCount); // Load GLTF animation data
#endif
#if defined(SUPPORT_FILEFORMAT_OBJ) || defined(SUPPORT_FILEFORMAT_MTL)
    static void sgProcessMaterialsOBJ(
        Material* materials,
        const std::vector<tinyobj::material_t>& mats,
        const std::string& path); // Process obj materials
#endif

    //----------------------------------------------------------------------------------
    // Module Functions Definition
    //----------------------------------------------------------------------------------

    // Load model from files (mesh and material)
    Model sgLoadModel(const char* fileName)
    {
        Model model{};

#if defined(SUPPORT_FILEFORMAT_OBJ)
        if (IsFileExtension(fileName, ".obj")) model = sgLoadOBJ(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
        if (IsFileExtension(fileName, ".gltf") || IsFileExtension(fileName, ".glb")) model = sgLoadGLTF(fileName);
#endif

        // Make sure model transform is set to identity matrix!
        model.transform = MatrixIdentity();

        if ((model.meshCount != 0) && (model.meshes != nullptr))
        {
            // Upload vertex data to GPU (static meshes)
            for (int i = 0; i < model.meshCount; i++)
                UploadMesh(&model.meshes[i], false);
        }
        else
            TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load model mesh(es) data", fileName);

        if (model.materialCount == 0)
        {
            TRACELOG(
                LOG_WARNING,
                "MATERIAL: [%s] Failed to load model material data, default to white material",
                fileName);

            model.materialCount = 1;
            model.materials = static_cast<Material*>(RL_CALLOC(model.materialCount, sizeof(Material)));
            model.materials[0] = LoadMaterialDefault();

            if (model.meshMaterial == nullptr)
                model.meshMaterial = static_cast<int*>(RL_CALLOC(model.meshCount, sizeof(int)));
        }

        return model;
    }

    // Load model from generated mesh
    // WARNING: A shallow copy of mesh is generated, passed by value,
    // as long as struct contains pointers to data and some values, we get a copy
    // of mesh pointing to same data as original version... be careful!
    Model sgLoadModelFromMesh(const Mesh& mesh)
    {
        Model model = {0};

        model.transform = MatrixIdentity();

        model.meshCount = 1;
        model.meshes = static_cast<Mesh*>(RL_CALLOC(model.meshCount, sizeof(Mesh)));
        model.meshes[0] = mesh;

        model.materialCount = 1;
        model.materials = static_cast<Material*>(RL_CALLOC(model.materialCount, sizeof(Material)));
        model.materials[0] = LoadMaterialDefault();

        model.meshMaterial = static_cast<int*>(RL_CALLOC(model.meshCount, sizeof(int)));
        model.meshMaterial[0] = 0; // First material index

        return model;
    }

    // Check if a model is valid (loaded in GPU, VAO/VBOs)
    bool sgIsModelValid(const Model& model)
    {
        bool result = false;

        if ((model.meshes != nullptr) &&       // Validate model contains some mesh
            (model.materials != nullptr) &&    // Validate model contains some material (at least default one)
            (model.meshMaterial != nullptr) && // Validate mesh-material linkage
            (model.meshCount > 0) &&           // Validate mesh count
            (model.materialCount > 0))
            result = true; // Validate material count

        // NOTE: Many elements could be validated from a model, including every model mesh VAO/VBOs
        // but some VBOs could not be used, it depends on Mesh vertex data
        for (int i = 0; i < model.meshCount; i++)
        {
            if ((model.meshes[i].vertices != nullptr) && (model.meshes[i].vboId[0] == 0))
            {
                result = false;
                break;
            } // Vertex position buffer not uploaded to GPU
            if ((model.meshes[i].texcoords != nullptr) && (model.meshes[i].vboId[1] == 0))
            {
                result = false;
                break;
            } // Vertex textcoords buffer not uploaded to GPU
            if ((model.meshes[i].normals != nullptr) && (model.meshes[i].vboId[2] == 0))
            {
                result = false;
                break;
            } // Vertex normals buffer not uploaded to GPU
            if ((model.meshes[i].colors != nullptr) && (model.meshes[i].vboId[3] == 0))
            {
                result = false;
                break;
            } // Vertex colors buffer not uploaded to GPU
            if ((model.meshes[i].tangents != nullptr) && (model.meshes[i].vboId[4] == 0))
            {
                result = false;
                break;
            } // Vertex tangents buffer not uploaded to GPU
            if ((model.meshes[i].texcoords2 != nullptr) && (model.meshes[i].vboId[5] == 0))
            {
                result = false;
                break;
            } // Vertex texcoords2 buffer not uploaded to GPU
            if ((model.meshes[i].indices != nullptr) && (model.meshes[i].vboId[6] == 0))
            {
                result = false;
                break;
            } // Vertex indices buffer not uploaded to GPU
            if ((model.meshes[i].boneIds != nullptr) && (model.meshes[i].vboId[7] == 0))
            {
                result = false;
                break;
            } // Vertex boneIds buffer not uploaded to GPU
            if ((model.meshes[i].boneWeights != nullptr) && (model.meshes[i].vboId[8] == 0))
            {
                result = false;
                break;
            } // Vertex boneWeights buffer not uploaded to GPU

            // NOTE: Some OpenGL versions do not support VAO, so we don't check it
            // if (model.meshes[i].vaoId == 0) { result = false; break }
        }

        return result;
    }

    //        // Upload vertex data into a VAO (if supported) and VBO
    //        void sgUploadMesh(Mesh* mesh, bool dynamic)
    //        {
    //            if (mesh->vaoId > 0)
    //            {
    //                // Check if mesh has already been loaded in GPU
    //                TRACELOG(LOG_WARNING, "VAO: [ID %i] Trying to re-load an already loaded mesh",
    //                mesh->vaoId); return;
    //            }
    //
    //            mesh->vboId = static_cast<unsigned int*>(RL_CALLOC(MAX_MESH_VERTEX_BUFFERS, sizeof(unsigned
    //            int)));
    //
    //            mesh->vaoId = 0;                                              // Vertex Array Object
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION] = 0;  // Vertex buffer: positions
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD] = 0;  // Vertex buffer: texcoords
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL] = 0;    // Vertex buffer: normals
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] = 0;     // Vertex buffer: colors
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT] = 0;   // Vertex buffer: tangents
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2] = 0; // Vertex buffer: texcoords2
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] = 0;   // Vertex buffer: indices
    //
    // #ifdef RL_SUPPORT_MESH_GPU_SKINNING
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS] = 0;     // Vertex buffer: boneIds
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS] = 0; // Vertex buffer: boneWeights
    // #endif
    //
    // #if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    //            mesh->vaoId = rlLoadVertexArray();
    //            rlEnableVertexArray(mesh->vaoId);
    //
    //            // NOTE: Vertex attributes must be uploaded considering default locations points and
    //            available vertex
    //            // data
    //
    //            // Enable vertex attributes: position (shader-location = 0)
    //            void* vertices = (mesh->animVertices != nullptr) ? mesh->animVertices : mesh->vertices;
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION] =
    //                rlLoadVertexBuffer(vertices, mesh->vertexCount * 3 * sizeof(float), dynamic);
    //            rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, false, 0, 0);
    //            rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
    //
    //            // Enable vertex attributes: texcoords (shader-location = 1)
    //            mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD] =
    //                rlLoadVertexBuffer(mesh->texcoords, mesh->vertexCount * 2 * sizeof(float), dynamic);
    //            rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, 2, RL_FLOAT, false, 0, 0);
    //            rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD);
    //
    //            // WARNING: When setting default vertex attribute values, the values for each generic vertex
    //            attribute
    //            // is part of current state, and it is maintained even if a different program object is used
    //
    //            if (mesh->normals != nullptr)
    //            {
    //                // Enable vertex attributes: normals (shader-location = 2)
    //                void* normals = (mesh->animNormals != nullptr) ? mesh->animNormals : mesh->normals;
    //                mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL] =
    //                    rlLoadVertexBuffer(normals, mesh->vertexCount * 3 * sizeof(float), dynamic);
    //                rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, 3, RL_FLOAT, false, 0, 0);
    //                rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);
    //            }
    //            else
    //            {
    //                // Default vertex attribute: normal
    //                // WARNING: Default value provided to shader if location available
    //                float value[3] = {1.0f, 1.0f, 1.0f};
    //                rlSetVertexAttributeDefault(
    //                    RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, value, SHADER_ATTRIB_VEC3, 3);
    //                rlDisableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);
    //            }
    //
    //            if (mesh->colors != nullptr)
    //            {
    //                // Enable vertex attribute: color (shader-location = 3)
    //                mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] =
    //                    rlLoadVertexBuffer(mesh->colors, mesh->vertexCount * 4 * sizeof(unsigned char),
    //                    dynamic);
    //                rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, 4, RL_UNSIGNED_BYTE, 1, 0,
    //                0); rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR);
    //            }
    //            else
    //            {
    //                // Default vertex attribute: color
    //                // WARNING: Default value provided to shader if location available
    //                float value[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // WHITE
    //                rlSetVertexAttributeDefault(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, value,
    //                SHADER_ATTRIB_VEC4, 4);
    //                rlDisableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR);
    //            }
    //
    //            if (mesh->tangents != nullptr)
    //            {
    //                // Enable vertex attribute: tangent (shader-location = 4)
    //                mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT] =
    //                    rlLoadVertexBuffer(mesh->tangents, mesh->vertexCount * 4 * sizeof(float), dynamic);
    //                rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT, 4, RL_FLOAT, 0, 0, 0);
    //                rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT);
    //            }
    //            else
    //            {
    //                // Default vertex attribute: tangent
    //                // WARNING: Default value provided to shader if location available
    //                float value[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    //                rlSetVertexAttributeDefault(
    //                    RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT, value, SHADER_ATTRIB_VEC4, 4);
    //                rlDisableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT);
    //            }
    //
    //            if (mesh->texcoords2 != nullptr)
    //            {
    //                // Enable vertex attribute: texcoord2 (shader-location = 5)
    //                mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2] =
    //                    rlLoadVertexBuffer(mesh->texcoords2, mesh->vertexCount * 2 * sizeof(float), dynamic);
    //                rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2, 2, RL_FLOAT, 0, 0, 0);
    //                rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2);
    //            }
    //            else
    //            {
    //                // Default vertex attribute: texcoord2
    //                // WARNING: Default value provided to shader if location available
    //                float value[2] = {0.0f, 0.0f};
    //                rlSetVertexAttributeDefault(
    //                    RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2, value, SHADER_ATTRIB_VEC2, 2);
    //                rlDisableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2);
    //            }
    //
    // #ifdef RL_SUPPORT_MESH_GPU_SKINNING
    //            if (mesh->boneIds != nullptr)
    //            {
    //                // Enable vertex attribute: boneIds (shader-location = 7)
    //                mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS] =
    //                    rlLoadVertexBuffer(mesh->boneIds, mesh->vertexCount * 4 * sizeof(unsigned char),
    //                    dynamic);
    //                rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS, 4, RL_UNSIGNED_BYTE, 0,
    //                0, 0); rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS);
    //            }
    //            else
    //            {
    //                // Default vertex attribute: boneIds
    //                // WARNING: Default value provided to shader if location available
    //                float value[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    //                rlSetVertexAttributeDefault(
    //                    RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS, value, SHADER_ATTRIB_VEC4, 4);
    //                rlDisableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS);
    //            }
    //
    //            if (mesh->boneWeights != nullptr)
    //            {
    //                // Enable vertex attribute: boneWeights (shader-location = 8)
    //                mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS] =
    //                    rlLoadVertexBuffer(mesh->boneWeights, mesh->vertexCount * 4 * sizeof(float),
    //                    dynamic);
    //                rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS, 4, RL_FLOAT, 0, 0,
    //                0); rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS);
    //            }
    //            else
    //            {
    //                // Default vertex attribute: boneWeights
    //                // WARNING: Default value provided to shader if location available
    //                float value[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    //                rlSetVertexAttributeDefault(
    //                    RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS, value, SHADER_ATTRIB_VEC4, 2);
    //                rlDisableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS);
    //            }
    // #endif
    //
    //            if (mesh->indices != nullptr)
    //            {
    //                mesh->vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] = rlLoadVertexBufferElement(
    //                    mesh->indices, mesh->triangleCount * 3 * sizeof(unsigned short), dynamic);
    //            }
    //
    //            if (mesh->vaoId > 0)
    //                TRACELOG(LOG_INFO, "VAO: [ID %i] Mesh uploaded successfully to VRAM (GPU)", mesh->vaoId);
    //            else
    //                TRACELOG(LOG_INFO, "VBO: Mesh uploaded successfully to VRAM (GPU)");
    //
    //            rlDisableVertexArray();
    // #endif
    //        }

    // Update mesh vertex data in GPU for a specific buffer index
    void sgUpdateMeshBuffer(Mesh mesh, int index, const void* data, int dataSize, int offset)
    {
        rlUpdateVertexBuffer(mesh.vboId[index], data, dataSize, offset);
    }

#if defined(SUPPORT_FILEFORMAT_OBJ) || defined(SUPPORT_FILEFORMAT_MTL)
    // Process obj materials
    static void sgProcessMaterialsOBJ(
        Material* materials, const std::vector<tinyobj::material_t>& mats, const std::string& path)
    {
        // Init model mats
        for (int m = 0; m < mats.size(); m++)
        {
            // Init material to default
            // NOTE: Uses default shader, which only supports MATERIAL_MAP_DIFFUSE
            materials[m] = LoadMaterialDefault();

            if (mats.empty()) return; // TODO: sus

            // Get default texture, in case no texture is defined
            // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
            materials[m].maps[MATERIAL_MAP_DIFFUSE].texture =
                Texture2D{rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};

            if (!mats[m].diffuse_texname.empty())
                materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(
                    std::string(path + mats[m].diffuse_texname).c_str()); // char *diffuse_texname; // map_Kd
            else
                materials[m].maps[MATERIAL_MAP_DIFFUSE].color = Color{
                    static_cast<unsigned char>(mats[m].diffuse[0] * 255.0f),
                    static_cast<unsigned char>(mats[m].diffuse[1] * 255.0f),
                    static_cast<unsigned char>(mats[m].diffuse[2] * 255.0f),
                    255}; // float diffuse[3];
            materials[m].maps[MATERIAL_MAP_DIFFUSE].value = 0.0f;

            if (!mats[m].specular_texname.empty())
                materials[m].maps[MATERIAL_MAP_SPECULAR].texture = LoadTexture(
                    std::string(path + mats[m].specular_texname).c_str()); // char *specular_texname; // map_Ks
            materials[m].maps[MATERIAL_MAP_SPECULAR].color = Color{
                static_cast<unsigned char>(mats[m].specular[0] * 255.0f),
                static_cast<unsigned char>(mats[m].specular[1] * 255.0f),
                static_cast<unsigned char>(mats[m].specular[2] * 255.0f),
                255}; // float specular[3];
            materials[m].maps[MATERIAL_MAP_SPECULAR].value = 0.0f;

            if (!mats[m].bump_texname.empty())
                materials[m].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture(
                    std::string(path + mats[m].bump_texname).c_str()); // char *bump_texname; // map_bump, bump
            materials[m].maps[MATERIAL_MAP_NORMAL].color = WHITE;
            materials[m].maps[MATERIAL_MAP_NORMAL].value = mats[m].shininess;

            if (!mats[m].emissive_texname.empty())
            {
                materials[m].maps[MATERIAL_MAP_EMISSION].texture = LoadTexture(
                    std::string(path + mats[m].emissive_texname).c_str()); // char *emissive_texname; // map_Ke
            }

            materials[m].maps[MATERIAL_MAP_EMISSION].color = Color{
                static_cast<unsigned char>(mats[m].emission[0] * 255.0f),
                static_cast<unsigned char>(mats[m].emission[1] * 255.0f),
                static_cast<unsigned char>(mats[m].emission[2] * 255.0f),
                255}; // float emission[3];

            if (!mats[m].displacement_texname.empty())
                materials[m].maps[MATERIAL_MAP_HEIGHT].texture =
                    LoadTexture(std::string(path + mats[m].displacement_texname)
                                    .c_str()); // char *displacement_texname; // disp
        }
    }
#endif

    // Check if a material is valid (map textures loaded in GPU)
    bool sgIsMaterialValid(Material material)
    {
        bool result = false;

        if ((material.maps != nullptr) && // Validate material contain some map
            (material.shader.id > 0))
            result = true; // Validate material shader is valid

        // TODO: Check if available maps contain loaded textures

        return result;
    }

    // Set texture for a material map type (MATERIAL_MAP_DIFFUSE, MATERIAL_MAP_SPECULAR...)
    // NOTE: Previous texture should be manually unloaded
    void sgSetMaterialTexture(Material* material, int mapType, Texture2D texture)
    {
        material->maps[mapType].texture = texture;
    }

    // Set the material for a mesh
    void sgSetModelMeshMaterial(Model* model, int meshId, int materialId)
    {
        if (meshId >= model->meshCount)
            TRACELOG(LOG_WARNING, "MESH: Id greater than mesh count");
        else if (materialId >= model->materialCount)
            TRACELOG(LOG_WARNING, "MATERIAL: Id greater than material count");
        else
            model->meshMaterial[meshId] = materialId;
    }

    // Load model animations from file
    ModelAnimation* sgLoadModelAnimations(const char* fileName, int* animCount)
    {
        ModelAnimation* animations = nullptr;

#if defined(SUPPORT_FILEFORMAT_GLTF)
        if (IsFileExtension(fileName, ".gltf;.glb")) animations = sgLoadModelAnimationsGLTF(fileName, animCount);
#endif

        return animations;
    }

    // Update model animated bones transform matrices for a given frame
    // NOTE: Updated data is not uploaded to GPU but kept at model.meshes[i].boneMatrices[boneId],
    // to be uploaded to shader at drawing, in case GPU skinning is enabled
    void sgUpdateModelAnimationBones(Model model, ModelAnimation anim, int frame)
    {
        if ((anim.frameCount > 0) && (anim.bones != nullptr) && (anim.framePoses != nullptr))
        {
            if (frame >= anim.frameCount) frame = frame % anim.frameCount;

            for (int i = 0; i < model.meshCount; i++)
            {
                if (model.meshes[i].boneMatrices)
                {
                    assert(model.meshes[i].boneCount == anim.boneCount);

                    for (int boneId = 0; boneId < model.meshes[i].boneCount; boneId++)
                    {
                        Vector3 inTranslation = model.bindPose[boneId].translation;
                        Quaternion inRotation = model.bindPose[boneId].rotation;
                        Vector3 inScale = model.bindPose[boneId].scale;

                        Vector3 outTranslation = anim.framePoses[frame][boneId].translation;
                        Quaternion outRotation = anim.framePoses[frame][boneId].rotation;
                        Vector3 outScale = anim.framePoses[frame][boneId].scale;

                        Vector3 invTranslation =
                            Vector3RotateByQuaternion(Vector3Negate(inTranslation), QuaternionInvert(inRotation));
                        Quaternion invRotation = QuaternionInvert(inRotation);
                        Vector3 invScale = Vector3Divide(Vector3{1.0f, 1.0f, 1.0f}, inScale);

                        Vector3 boneTranslation = Vector3Add(
                            Vector3RotateByQuaternion(Vector3Multiply(outScale, invTranslation), outRotation),
                            outTranslation);
                        Quaternion boneRotation = QuaternionMultiply(outRotation, invRotation);
                        Vector3 boneScale = Vector3Multiply(outScale, invScale);

                        Matrix boneMatrix = MatrixMultiply(
                            MatrixMultiply(
                                QuaternionToMatrix(boneRotation),
                                MatrixTranslate(boneTranslation.x, boneTranslation.y, boneTranslation.z)),
                            MatrixScale(boneScale.x, boneScale.y, boneScale.z));

                        model.meshes[i].boneMatrices[boneId] = boneMatrix;
                    }
                }
            }
        }
    }

    //----------------------------------------------------------------------------------
    // Module specific Functions Definition
    //----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_IQM) || defined(SUPPORT_FILEFORMAT_GLTF)
    // Build pose from parent joints
    // NOTE: Required for animations loading (required by IQM and GLTF)
    static void sgBuildPoseFromParentJoints(BoneInfo* bones, int boneCount, Transform* transforms)
    {
        for (int i = 0; i < boneCount; i++)
        {
            if (bones[i].parent >= 0)
            {
                if (bones[i].parent > i)
                {
                    TRACELOG(
                        LOG_WARNING,
                        "Assumes bones are toplogically sorted, but bone %d has parent %d. Skipping.",
                        i,
                        bones[i].parent);
                    continue;
                }
                transforms[i].rotation =
                    QuaternionMultiply(transforms[bones[i].parent].rotation, transforms[i].rotation);
                transforms[i].translation =
                    Vector3RotateByQuaternion(transforms[i].translation, transforms[bones[i].parent].rotation);
                transforms[i].translation =
                    Vector3Add(transforms[i].translation, transforms[bones[i].parent].translation);
                transforms[i].scale = Vector3Multiply(transforms[i].scale, transforms[bones[i].parent].scale);
            }
        }
    }
#endif

#if defined(SUPPORT_FILEFORMAT_OBJ)
    // Load OBJ mesh data
    //
    // Keep the following information in mind when reading this
    //  - A mesh is created for every material present in the obj file
    //  - the model.meshCount is therefore the materialCount returned from tinyobj
    //  - the mesh is automatically triangulated by tinyobj
    static Model sgLoadOBJ(const char* fileName)
    {
        Model model{};
        model.transform = MatrixIdentity();

        auto path = std::string(GetDirectoryPath(fileName)) + std::string("/");

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName, path.c_str(), true, true);

        if (!warn.empty()) TRACELOG(LOG_WARNING, "MODEL: %s", warn.c_str());
        if (!err.empty()) TRACELOG(LOG_ERROR, "MODEL: %s", err.c_str());
        if (!ret) return model;

        // Setup model with material-based meshes
        model.materialCount = static_cast<int>(materials.empty() ? 1 : materials.size());
        model.meshCount = model.materialCount;
        model.meshes = static_cast<Mesh*>(MemAlloc(sizeof(Mesh) * model.meshCount));
        model.materials = static_cast<Material*>(MemAlloc(sizeof(Material) * model.materialCount));
        model.meshMaterial = static_cast<int*>(MemAlloc(sizeof(int) * model.meshCount));

        // Count vertices per material
        std::vector<size_t> verticesPerMaterial(model.materialCount, 0);
        for (const auto& shape : shapes)
        {
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                int matId = shape.mesh.material_ids[f];
                if (matId < 0) matId = 0;
                verticesPerMaterial[matId] += shape.mesh.num_face_vertices[f];
            }
        }

        // Allocate mesh buffers
        for (int m = 0; m < model.meshCount; m++)
        {
            model.meshes[m].vertexCount = verticesPerMaterial[m];
            model.meshes[m].triangleCount = verticesPerMaterial[m] / 3;
            model.meshes[m].vertices = static_cast<float*>(MemAlloc(sizeof(float) * verticesPerMaterial[m] * 3));
            model.meshes[m].normals = static_cast<float*>(MemAlloc(sizeof(float) * verticesPerMaterial[m] * 3));
            model.meshes[m].texcoords = static_cast<float*>(MemAlloc(sizeof(float) * verticesPerMaterial[m] * 2));
            model.meshes[m].colors =
                static_cast<unsigned char*>(MemAlloc(sizeof(unsigned char) * verticesPerMaterial[m] * 4));
            model.meshMaterial[m] = m;
        }

        // Fill mesh data
        std::vector<size_t> currentVertex(model.meshCount, 0);
        for (const auto& shape : shapes)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                int matId = shape.mesh.material_ids[f];
                if (matId < 0) matId = 0;

                size_t fv = shape.mesh.num_face_vertices[f];
                for (size_t v = 0; v < fv; v++)
                {
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                    size_t vIdx = currentVertex[matId];

                    // Vertices
                    for (int k = 0; k < 3; k++)
                    {
                        model.meshes[matId].vertices[vIdx * 3 + k] = attrib.vertices[3 * idx.vertex_index + k];
                    }

                    // Normals
                    if (idx.normal_index >= 0)
                    {
                        for (int k = 0; k < 3; k++)
                        {
                            model.meshes[matId].normals[vIdx * 3 + k] = attrib.normals[3 * idx.normal_index + k];
                        }
                    }

                    // Texcoords
                    if (idx.texcoord_index >= 0)
                    {
                        model.meshes[matId].texcoords[vIdx * 2] = attrib.texcoords[2 * idx.texcoord_index];
                        model.meshes[matId].texcoords[vIdx * 2 + 1] =
                            1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
                    }

                    // Colors (default white)
                    for (int k = 0; k < 4; k++)
                    {
                        model.meshes[matId].colors[vIdx * 4 + k] = 255;
                    }

                    currentVertex[matId]++;
                }
                index_offset += fv;
            }
        }

        // Process materials
        if (!materials.empty())
        {
            sgProcessMaterialsOBJ(model.materials, materials, path);
        }
        else
        {
            model.materials[0] = LoadMaterialDefault();
        }

        // Upload mesh data to GPU
        for (int i = 0; i < model.meshCount; i++)
        {
            UploadMesh(&model.meshes[i], true);
        }

        return model;
    }
#endif

#if defined(SUPPORT_FILEFORMAT_GLTF)
    // Load file data callback for cgltf
    static cgltf_result sgLoadFileGLTFCallback(
        const struct cgltf_memory_options* memoryOptions,
        const struct cgltf_file_options* fileOptions,
        const char* path,
        cgltf_size* size,
        void** data)
    {
        int filesize;
        unsigned char* filedata = LoadFileData(path, &filesize);

        if (filedata == nullptr) return cgltf_result_io_error;

        *size = filesize;
        *data = filedata;

        return cgltf_result_success;
    }

    // Release file data callback for cgltf
    static void sgReleaseFileGLTFCallback(
        const struct cgltf_memory_options* memoryOptions, const struct cgltf_file_options* fileOptions, void* data)
    {
        UnloadFileData((unsigned char*)data);
    }

    // Load image from different glTF provided methods (uri, path, buffer_view)
    static Image sgLoadImageFromCgltfImage(cgltf_image* cgltfImage, const char* texPath)
    {
        Image image{};

        if (cgltfImage->uri != nullptr) // Check if image data is provided as an uri (base64 or path)
        {
            if ((strlen(cgltfImage->uri) > 5) && (cgltfImage->uri[0] == 'd') && (cgltfImage->uri[1] == 'a') &&
                (cgltfImage->uri[2] == 't') && (cgltfImage->uri[3] == 'a') &&
                (cgltfImage->uri[4] == ':')) // Check if image is provided as base64 text data
            {
                // Data URI Format: data:<mediatype>;base64,<data>

                // Find the comma
                int i = 0;
                while ((cgltfImage->uri[i] != ',') && (cgltfImage->uri[i] != 0))
                    i++;

                if (cgltfImage->uri[i] == 0)
                    TRACELOG(LOG_WARNING, "IMAGE: glTF data URI is not a valid image");
                else
                {
                    int base64Size = (int)strlen(cgltfImage->uri + i + 1);
                    while (cgltfImage->uri[i + base64Size] == '=')
                        base64Size--; // Ignore optional paddings
                    int numberOfEncodedBits =
                        base64Size * 6 -
                        (base64Size * 6) % 8; // Encoded bits minus extra bits, so it becomes a multiple of 8 bits
                    int outSize = numberOfEncodedBits / 8; // Actual encoded bytes
                    void* data = nullptr;

                    cgltf_options options{};
                    options.file.read = sgLoadFileGLTFCallback;
                    options.file.release = sgReleaseFileGLTFCallback;
                    cgltf_result result =
                        cgltf_load_buffer_base64(&options, outSize, cgltfImage->uri + i + 1, &data);

                    if (result == cgltf_result_success)
                    {
                        image = LoadImageFromMemory(".png", (unsigned char*)data, outSize);
                        RL_FREE(data);
                    }
                }
            }
            else // Check if image is provided as image path
            {
                image = LoadImage(TextFormat("%s/%s", texPath, cgltfImage->uri));
            }
        }
        else if (cgltfImage->buffer_view->buffer->data != nullptr) // Check if image is provided as data buffer
        {
            auto data = (unsigned char*)RL_MALLOC(cgltfImage->buffer_view->size);
            int offset = (int)cgltfImage->buffer_view->offset;
            int stride = (int)cgltfImage->buffer_view->stride ? (int)cgltfImage->buffer_view->stride : 1;

            // Copy buffer data to memory for loading
            for (unsigned int i = 0; i < cgltfImage->buffer_view->size; i++)
            {
                data[i] = ((unsigned char*)cgltfImage->buffer_view->buffer->data)[offset];
                offset += stride;
            }

            // Check mime_type for image: (cgltfImage->mime_type == "image/png")
            // NOTE: Detected that some models define mime_type as "image\\/png"
            if ((strcmp(cgltfImage->mime_type, "image\\/png") == 0) ||
                (strcmp(cgltfImage->mime_type, "image/png") == 0))
                image = LoadImageFromMemory(".png", data, (int)cgltfImage->buffer_view->size);
            else if (
                (strcmp(cgltfImage->mime_type, "image\\/jpeg") == 0) ||
                (strcmp(cgltfImage->mime_type, "image/jpeg") == 0))
                image = LoadImageFromMemory(".jpg", data, (int)cgltfImage->buffer_view->size);
            else
                TRACELOG(
                    LOG_WARNING,
                    "MODEL: glTF image data MIME type not recognized",
                    TextFormat("%s/%s", texPath, cgltfImage->uri));

            RL_FREE(data);
        }

        return image;
    }

    // Load bone info from GLTF skin data
    static BoneInfo* sgLoadBoneInfoGLTF(cgltf_skin skin, int* boneCount)
    {
        *boneCount = (int)skin.joints_count;
        auto bones = (BoneInfo*)RL_MALLOC(skin.joints_count * sizeof(BoneInfo));

        for (unsigned int i = 0; i < skin.joints_count; i++)
        {
            cgltf_node node = *skin.joints[i];
            if (node.name != nullptr)
            {
                strncpy(bones[i].name, node.name, sizeof(bones[i].name));
                bones[i].name[sizeof(bones[i].name) - 1] = '\0';
            }

            // Find parent bone index
            int parentIndex = -1;

            for (unsigned int j = 0; j < skin.joints_count; j++)
            {
                if (skin.joints[j] == node.parent)
                {
                    parentIndex = (int)j;
                    break;
                }
            }

            bones[i].parent = parentIndex;
        }

        return bones;
    }

    // Load glTF file into model struct, .gltf and .glb supported
    static Model sgLoadGLTF(const char* fileName)
    {
        /*********************************************************************************************

            Function implemented by Wilhem Barbier(@wbrbr), with modifications by Tyler Bezera(@gamerfiend)
            Transform handling implemented by Paul Melis (@paulmelis).
            Reviewed by Ramon Santamaria (@raysan5)

            FEATURES:
              - Supports .gltf and .glb files
              - Supports embedded (base64) or external textures
              - Supports PBR metallic/roughness flow, loads material textures, values and colors
                         PBR specular/glossiness flow and extended texture flows not supported
              - Supports multiple meshes per model (every primitives is loaded as a separate mesh)
              - Supports basic animations
              - Transforms, including parent-child relations, are applied on the mesh data, but the
                hierarchy is not kept (as it can't be represented).
              - Mesh instances in the glTF file (i.e. same mesh linked from multiple nodes)
                are turned into separate raylib Meshes.

            RESTRICTIONS:
              - Only triangle meshes supported
              - Vertex attribute types and formats supported:
                  > Vertices (position): vec3: float
                  > Normals: vec3: float
                  > Texcoords: vec2: float
                  > Colors: vec4: u8, u16, f32 (normalized)
                  > Indices: u16, u32 (truncated to u16)
              - Scenes defined in the glTF file are ignored. All nodes in the file
                are used.

        ***********************************************************************************************/

        // Macro to simplify attributes loading code
#define LOAD_ATTRIBUTE(accesor, numComp, srcType, dstPtr)                                                         \
    LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, srcType)

#define LOAD_ATTRIBUTE_CAST(accesor, numComp, srcType, dstPtr, dstType)                                           \
    {                                                                                                             \
        int n = 0;                                                                                                \
        srcType* buffer = (srcType*)accesor->buffer_view->buffer->data +                                          \
                          accesor->buffer_view->offset / sizeof(srcType) + accesor->offset / sizeof(srcType);     \
        for (unsigned int k = 0; k < accesor->count; k++)                                                         \
        {                                                                                                         \
            for (int l = 0; l < numComp; l++)                                                                     \
            {                                                                                                     \
                dstPtr[numComp * k + l] = (dstType)buffer[n + l];                                                 \
            }                                                                                                     \
            n += (int)(accesor->stride / sizeof(srcType));                                                        \
        }                                                                                                         \
    }

        Model model = {0};

        // glTF file loading
        int dataSize = 0;
        unsigned char* fileData = LoadFileData(fileName, &dataSize);

        if (fileData == nullptr) return model;

        // glTF data loading
        cgltf_options options{};
        options.file.read = sgLoadFileGLTFCallback;
        options.file.release = sgReleaseFileGLTFCallback;
        cgltf_data* data = nullptr;
        cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

        if (result == cgltf_result_success)
        {
            if (data->file_type == cgltf_file_type_glb)
                TRACELOG(LOG_INFO, "MODEL: [%s] Model basic data (glb) loaded successfully", fileName);
            else if (data->file_type == cgltf_file_type_gltf)
                TRACELOG(LOG_INFO, "MODEL: [%s] Model basic data (glTF) loaded successfully", fileName);
            else
                TRACELOG(LOG_WARNING, "MODEL: [%s] Model format not recognized", fileName);

            TRACELOG(LOG_INFO, "    > Meshes count: %i", data->meshes_count);
            TRACELOG(LOG_INFO, "    > Materials count: %i (+1 default)", data->materials_count);
            TRACELOG(LOG_DEBUG, "    > Buffers count: %i", data->buffers_count);
            TRACELOG(LOG_DEBUG, "    > Images count: %i", data->images_count);
            TRACELOG(LOG_DEBUG, "    > Textures count: %i", data->textures_count);

            // Force reading data buffers (fills buffer_view->buffer->data)
            // NOTE: If an uri is defined to base64 data or external path, it's automatically loaded
            result = cgltf_load_buffers(&options, data, fileName);
            if (result != cgltf_result_success)
                TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load mesh/material buffers", fileName);

            int primitivesCount = 0;
            // NOTE: We will load every primitive in the glTF as a separate raylib Mesh.
            // Determine total number of meshes needed from the node hierarchy.
            for (unsigned int i = 0; i < data->nodes_count; i++)
            {
                cgltf_node* node = &(data->nodes[i]);
                cgltf_mesh* mesh = node->mesh;
                if (!mesh) continue;

                for (unsigned int p = 0; p < mesh->primitives_count; p++)
                {
                    if (mesh->primitives[p].type == cgltf_primitive_type_triangles) primitivesCount++;
                }
            }
            TRACELOG(
                LOG_DEBUG, "    > Primitives (triangles only) count based on hierarchy : %i", primitivesCount);

            // Load our model data: meshes and materials
            model.meshCount = primitivesCount;
            model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));

            // NOTE: We keep an extra slot for default material, in case some mesh requires it
            model.materialCount = (int)data->materials_count + 1;
            model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
            model.materials[0] = LoadMaterialDefault(); // Load default material (index: 0)

            // Load mesh-material indices, by default all meshes are mapped to material index: 0
            model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));

            // Load materials data
            //----------------------------------------------------------------------------------------------------
            for (unsigned int i = 0, j = 1; i < data->materials_count; i++, j++)
            {
                model.materials[j] = LoadMaterialDefault();
                const char* texPath = GetDirectoryPath(fileName);

                // Check glTF material flow: PBR metallic/roughness flow
                // NOTE: Alternatively, materials can follow PBR specular/glossiness flow
                if (data->materials[i].has_pbr_metallic_roughness)
                {
                    // Load base color texture (albedo)
                    if (data->materials[i].pbr_metallic_roughness.base_color_texture.texture)
                    {
                        Image imAlbedo = sgLoadImageFromCgltfImage(
                            data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image, texPath);
                        if (imAlbedo.data != nullptr)
                        {
                            model.materials[j].maps[MATERIAL_MAP_ALBEDO].texture = LoadTextureFromImage(imAlbedo);
                            UnloadImage(imAlbedo);
                        }
                    }
                    // Load base color factor (tint)
                    model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.r =
                        (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[0] * 255);
                    model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.g =
                        (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[1] * 255);
                    model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.b =
                        (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[2] * 255);
                    model.materials[j].maps[MATERIAL_MAP_ALBEDO].color.a =
                        (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[3] * 255);

                    // Load metallic/roughness texture
                    if (data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture)
                    {
                        Image imMetallicRoughness = sgLoadImageFromCgltfImage(
                            data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture->image,
                            texPath);
                        if (imMetallicRoughness.data != nullptr)
                        {
                            model.materials[j].maps[MATERIAL_MAP_ROUGHNESS].texture =
                                LoadTextureFromImage(imMetallicRoughness);
                            UnloadImage(imMetallicRoughness);
                        }

                        // Load metallic/roughness material properties
                        float roughness = data->materials[i].pbr_metallic_roughness.roughness_factor;
                        model.materials[j].maps[MATERIAL_MAP_ROUGHNESS].value = roughness;

                        float metallic = data->materials[i].pbr_metallic_roughness.metallic_factor;
                        model.materials[j].maps[MATERIAL_MAP_METALNESS].value = metallic;
                    }

                    // Load normal texture
                    if (data->materials[i].normal_texture.texture)
                    {
                        Image imNormal =
                            sgLoadImageFromCgltfImage(data->materials[i].normal_texture.texture->image, texPath);
                        if (imNormal.data != nullptr)
                        {
                            model.materials[j].maps[MATERIAL_MAP_NORMAL].texture = LoadTextureFromImage(imNormal);
                            UnloadImage(imNormal);
                        }
                    }

                    // Load ambient occlusion texture
                    if (data->materials[i].occlusion_texture.texture)
                    {
                        Image imOcclusion = sgLoadImageFromCgltfImage(
                            data->materials[i].occlusion_texture.texture->image, texPath);
                        if (imOcclusion.data != nullptr)
                        {
                            model.materials[j].maps[MATERIAL_MAP_OCCLUSION].texture =
                                LoadTextureFromImage(imOcclusion);
                            UnloadImage(imOcclusion);
                        }
                    }

                    // Load emissive texture
                    if (data->materials[i].emissive_texture.texture)
                    {
                        Image imEmissive =
                            sgLoadImageFromCgltfImage(data->materials[i].emissive_texture.texture->image, texPath);
                        if (imEmissive.data != nullptr)
                        {
                            model.materials[j].maps[MATERIAL_MAP_EMISSION].texture =
                                LoadTextureFromImage(imEmissive);
                            UnloadImage(imEmissive);
                        }

                        // Load emissive color factor
                        model.materials[j].maps[MATERIAL_MAP_EMISSION].color.r =
                            static_cast<unsigned char>(data->materials[i].emissive_factor[0] * 255);
                        model.materials[j].maps[MATERIAL_MAP_EMISSION].color.g =
                            static_cast<unsigned char>(data->materials[i].emissive_factor[1] * 255);
                        model.materials[j].maps[MATERIAL_MAP_EMISSION].color.b =
                            static_cast<unsigned char>(data->materials[i].emissive_factor[2] * 255);
                        model.materials[j].maps[MATERIAL_MAP_EMISSION].color.a = 255;
                    }
                }

                // Other possible materials not supported by raylib pipeline:
                // has_clearcoat, has_transmission, has_volume, has_ior, has specular, has_sheen
            }

            // Visit each node in the hierarchy and process any mesh linked from it.
            // Each primitive within a glTF node becomes a Raylib Mesh.
            // The local-to-world transform of each node is used to transform the
            // points/normals/tangents of the created Mesh(es).
            // Any glTF mesh linked from more than one Node (i.e. instancing)
            // is turned into multiple Mesh's, as each Node will have its own
            // transform applied.
            // Note: the code below disregards the scenes defined in the file, all nodes are used.
            //----------------------------------------------------------------------------------------------------
            int meshIndex = 0;
            for (unsigned int i = 0; i < data->nodes_count; i++)
            {
                cgltf_node* node = &(data->nodes[i]);

                cgltf_mesh* mesh = node->mesh;
                if (!mesh) continue;

                cgltf_float worldTransform[16];
                cgltf_node_transform_world(node, worldTransform);

                Matrix worldMatrix = {
                    worldTransform[0],
                    worldTransform[4],
                    worldTransform[8],
                    worldTransform[12],
                    worldTransform[1],
                    worldTransform[5],
                    worldTransform[9],
                    worldTransform[13],
                    worldTransform[2],
                    worldTransform[6],
                    worldTransform[10],
                    worldTransform[14],
                    worldTransform[3],
                    worldTransform[7],
                    worldTransform[11],
                    worldTransform[15]};

                Matrix worldMatrixNormals = MatrixTranspose(MatrixInvert(worldMatrix));

                for (unsigned int p = 0; p < mesh->primitives_count; p++)
                {
                    // NOTE: We only support primitives defined by triangles
                    // Other alternatives: points, lines, line_strip, triangle_strip
                    if (mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;

                    // NOTE: Attributes data could be provided in several data formats (8, 8u, 16u, 32...),
                    // Only some formats for each attribute type are supported, read info at the top of this
                    // function!

                    for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
                    {
                        // Check the different attributes for every primitive
                        if (mesh->primitives[p].attributes[j].type ==
                            cgltf_attribute_type_position) // POSITION, vec3, float
                        {
                            cgltf_accessor* attribute = mesh->primitives[p].attributes[j].data;

                            // WARNING: SPECS: POSITION accessor MUST have its min and max properties defined

                            if ((attribute->type == cgltf_type_vec3) &&
                                (attribute->component_type == cgltf_component_type_r_32f))
                            {
                                // Init raylib mesh vertices to copy glTF attribute data
                                model.meshes[meshIndex].vertexCount = (int)attribute->count;
                                model.meshes[meshIndex].vertices =
                                    (float*)RL_MALLOC(attribute->count * 3 * sizeof(float));

                                // Load 3 components of float data type into mesh.vertices
                                LOAD_ATTRIBUTE(attribute, 3, float, model.meshes[meshIndex].vertices)

                                // Transform the vertices
                                float* vertices = model.meshes[meshIndex].vertices;
                                for (unsigned int k = 0; k < attribute->count; k++)
                                {
                                    Vector3 vt = Vector3Transform(
                                        Vector3{vertices[3 * k], vertices[3 * k + 1], vertices[3 * k + 2]},
                                        worldMatrix);
                                    vertices[3 * k] = vt.x;
                                    vertices[3 * k + 1] = vt.y;
                                    vertices[3 * k + 2] = vt.z;
                                }
                            }
                            else
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Vertices attribute data format not supported, use vec3 float",
                                    fileName);
                        }
                        else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_normal) // NORMAL,
                                                                                                        // vec3,
                                                                                                        // float
                        {
                            cgltf_accessor* attribute = mesh->primitives[p].attributes[j].data;

                            if ((attribute->type == cgltf_type_vec3) &&
                                (attribute->component_type == cgltf_component_type_r_32f))
                            {
                                // Init raylib mesh normals to copy glTF attribute data
                                model.meshes[meshIndex].normals =
                                    (float*)RL_MALLOC(attribute->count * 3 * sizeof(float));

                                // Load 3 components of float data type into mesh.normals
                                LOAD_ATTRIBUTE(attribute, 3, float, model.meshes[meshIndex].normals)

                                // Transform the normals
                                float* normals = model.meshes[meshIndex].normals;
                                for (unsigned int k = 0; k < attribute->count; k++)
                                {
                                    Vector3 nt = Vector3Transform(
                                        Vector3{normals[3 * k], normals[3 * k + 1], normals[3 * k + 2]},
                                        worldMatrixNormals);
                                    normals[3 * k] = nt.x;
                                    normals[3 * k + 1] = nt.y;
                                    normals[3 * k + 2] = nt.z;
                                }
                            }
                            else
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Normal attribute data format not supported, use vec3 float",
                                    fileName);
                        }
                        else if (
                            mesh->primitives[p].attributes[j].type == cgltf_attribute_type_tangent) // TANGENT,
                                                                                                    // vec3,
                                                                                                    // float
                        {
                            cgltf_accessor* attribute = mesh->primitives[p].attributes[j].data;

                            if ((attribute->type == cgltf_type_vec4) &&
                                (attribute->component_type == cgltf_component_type_r_32f))
                            {
                                // Init raylib mesh tangent to copy glTF attribute data
                                model.meshes[meshIndex].tangents =
                                    (float*)RL_MALLOC(attribute->count * 4 * sizeof(float));

                                // Load 4 components of float data type into mesh.tangents
                                LOAD_ATTRIBUTE(attribute, 4, float, model.meshes[meshIndex].tangents)

                                // Transform the tangents
                                float* tangents = model.meshes[meshIndex].tangents;
                                for (unsigned int k = 0; k < attribute->count; k++)
                                {
                                    Vector3 tt = Vector3Transform(
                                        Vector3{tangents[3 * k], tangents[3 * k + 1], tangents[3 * k + 2]},
                                        worldMatrix);
                                    tangents[3 * k] = tt.x;
                                    tangents[3 * k + 1] = tt.y;
                                    tangents[3 * k + 2] = tt.z;
                                }
                            }
                            else
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Tangent attribute data format not supported, use vec4 float",
                                    fileName);
                        }
                        else if (
                            mesh->primitives[p].attributes[j].type ==
                            cgltf_attribute_type_texcoord) // TEXCOORD_n, vec2, float/u8n/u16n
                        {
                            // Support up to 2 texture coordinates attributes
                            float* texcoordPtr = nullptr;

                            cgltf_accessor* attribute = mesh->primitives[p].attributes[j].data;

                            if (attribute->type == cgltf_type_vec2)
                            {
                                if (attribute->component_type == cgltf_component_type_r_32f) // vec2, float
                                {
                                    // Init raylib mesh texcoords to copy glTF attribute data
                                    texcoordPtr = (float*)RL_MALLOC(attribute->count * 2 * sizeof(float));

                                    // Load 3 components of float data type into mesh.texcoords
                                    LOAD_ATTRIBUTE(attribute, 2, float, texcoordPtr)
                                }
                                else if (attribute->component_type == cgltf_component_type_r_8u) // vec2, u8n
                                {
                                    // Init raylib mesh texcoords to copy glTF attribute data
                                    texcoordPtr = (float*)RL_MALLOC(attribute->count * 2 * sizeof(float));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp =
                                        (unsigned char*)RL_MALLOC(attribute->count * 2 * sizeof(unsigned char));
                                    LOAD_ATTRIBUTE(attribute, 2, unsigned char, temp);

                                    // Convert data to raylib texcoord data type (float)
                                    for (unsigned int t = 0; t < attribute->count * 2; t++)
                                        texcoordPtr[t] = (float)temp[t] / 255.0f;

                                    RL_FREE(temp);
                                }
                                else if (attribute->component_type == cgltf_component_type_r_16u) // vec2, u16n
                                {
                                    // Init raylib mesh texcoords to copy glTF attribute data
                                    texcoordPtr = (float*)RL_MALLOC(attribute->count * 2 * sizeof(float));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp =
                                        (unsigned short*)RL_MALLOC(attribute->count * 2 * sizeof(unsigned short));
                                    LOAD_ATTRIBUTE(attribute, 2, unsigned short, temp);

                                    // Convert data to raylib texcoord data type (float)
                                    for (unsigned int t = 0; t < attribute->count * 2; t++)
                                        texcoordPtr[t] = (float)temp[t] / 65535.0f;

                                    RL_FREE(temp);
                                }
                                else
                                    TRACELOG(
                                        LOG_WARNING,
                                        "MODEL: [%s] Texcoords attribute data format not supported",
                                        fileName);
                            }
                            else
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Texcoords attribute data format not supported, use vec2 "
                                    "float",
                                    fileName);

                            int index = mesh->primitives[p].attributes[j].index;
                            if (index == 0)
                                model.meshes[meshIndex].texcoords = texcoordPtr;
                            else if (index == 1)
                                model.meshes[meshIndex].texcoords2 = texcoordPtr;
                            else
                            {
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] No more than 2 texture coordinates attributes supported",
                                    fileName);
                                if (texcoordPtr != nullptr) RL_FREE(texcoordPtr);
                            }
                        }
                        else if (
                            mesh->primitives[p].attributes[j].type ==
                            cgltf_attribute_type_color) // COLOR_n, vec3/vec4, float/u8n/u16n
                        {
                            cgltf_accessor* attribute = mesh->primitives[p].attributes[j].data;

                            // WARNING: SPECS: All components of each COLOR_n accessor element MUST be clamped
                            // to [0.0, 1.0] range

                            if (attribute->type == cgltf_type_vec3) // RGB
                            {
                                if (attribute->component_type == cgltf_component_type_r_8u)
                                {
                                    // Init raylib mesh color to copy glTF attribute data
                                    model.meshes[meshIndex].colors =
                                        (unsigned char*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned char));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp =
                                        (unsigned char*)RL_MALLOC(attribute->count * 3 * sizeof(unsigned char));
                                    LOAD_ATTRIBUTE(attribute, 3, unsigned char, temp);

                                    // Convert data to raylib color data type (4 bytes)
                                    for (unsigned int c = 0, k = 0; c < (attribute->count * 4 - 3); c += 4, k += 3)
                                    {
                                        model.meshes[meshIndex].colors[c] = temp[k];
                                        model.meshes[meshIndex].colors[c + 1] = temp[k + 1];
                                        model.meshes[meshIndex].colors[c + 2] = temp[k + 2];
                                        model.meshes[meshIndex].colors[c + 3] = 255;
                                    }

                                    RL_FREE(temp);
                                }
                                else if (attribute->component_type == cgltf_component_type_r_16u)
                                {
                                    // Init raylib mesh color to copy glTF attribute data
                                    model.meshes[meshIndex].colors =
                                        (unsigned char*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned char));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp =
                                        (unsigned short*)RL_MALLOC(attribute->count * 3 * sizeof(unsigned short));
                                    LOAD_ATTRIBUTE(attribute, 3, unsigned short, temp);

                                    // Convert data to raylib color data type (4 bytes)
                                    for (unsigned int c = 0, k = 0; c < (attribute->count * 4 - 3); c += 4, k += 3)
                                    {
                                        model.meshes[meshIndex].colors[c] =
                                            (unsigned char)(((float)temp[k] / 65535.0f) * 255.0f);
                                        model.meshes[meshIndex].colors[c + 1] =
                                            (unsigned char)(((float)temp[k + 1] / 65535.0f) * 255.0f);
                                        model.meshes[meshIndex].colors[c + 2] =
                                            (unsigned char)(((float)temp[k + 2] / 65535.0f) * 255.0f);
                                        model.meshes[meshIndex].colors[c + 3] = 255;
                                    }

                                    RL_FREE(temp);
                                }
                                else if (attribute->component_type == cgltf_component_type_r_32f)
                                {
                                    // Init raylib mesh color to copy glTF attribute data
                                    model.meshes[meshIndex].colors =
                                        (unsigned char*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned char));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp = (float*)RL_MALLOC(attribute->count * 3 * sizeof(float));
                                    LOAD_ATTRIBUTE(attribute, 3, float, temp);

                                    // Convert data to raylib color data type (4 bytes)
                                    for (unsigned int c = 0, k = 0; c < (attribute->count * 4 - 3); c += 4, k += 3)
                                    {
                                        model.meshes[meshIndex].colors[c] = (unsigned char)(temp[k] * 255.0f);
                                        model.meshes[meshIndex].colors[c + 1] =
                                            (unsigned char)(temp[k + 1] * 255.0f);
                                        model.meshes[meshIndex].colors[c + 2] =
                                            (unsigned char)(temp[k + 2] * 255.0f);
                                        model.meshes[meshIndex].colors[c + 3] = 255;
                                    }

                                    RL_FREE(temp);
                                }
                                else
                                    TRACELOG(
                                        LOG_WARNING,
                                        "MODEL: [%s] Color attribute data format not supported",
                                        fileName);
                            }
                            else if (attribute->type == cgltf_type_vec4) // RGBA
                            {
                                if (attribute->component_type == cgltf_component_type_r_8u)
                                {
                                    // Init raylib mesh color to copy glTF attribute data
                                    model.meshes[meshIndex].colors =
                                        (unsigned char*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned char));

                                    // Load 4 components of unsigned char data type into mesh.colors
                                    LOAD_ATTRIBUTE(attribute, 4, unsigned char, model.meshes[meshIndex].colors)
                                }
                                else if (attribute->component_type == cgltf_component_type_r_16u)
                                {
                                    // Init raylib mesh color to copy glTF attribute data
                                    model.meshes[meshIndex].colors =
                                        (unsigned char*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned char));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp =
                                        (unsigned short*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned short));
                                    LOAD_ATTRIBUTE(attribute, 4, unsigned short, temp);

                                    // Convert data to raylib color data type (4 bytes)
                                    for (unsigned int c = 0; c < attribute->count * 4; c++)
                                        model.meshes[meshIndex].colors[c] =
                                            (unsigned char)(((float)temp[c] / 65535.0f) * 255.0f);

                                    RL_FREE(temp);
                                }
                                else if (attribute->component_type == cgltf_component_type_r_32f)
                                {
                                    // Init raylib mesh color to copy glTF attribute data
                                    model.meshes[meshIndex].colors =
                                        (unsigned char*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned char));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp = (float*)RL_MALLOC(attribute->count * 4 * sizeof(float));
                                    LOAD_ATTRIBUTE(attribute, 4, float, temp);

                                    // Convert data to raylib color data type (4 bytes), we expect the color
                                    // data normalized
                                    for (unsigned int c = 0; c < attribute->count * 4; c++)
                                        model.meshes[meshIndex].colors[c] = (unsigned char)(temp[c] * 255.0f);

                                    RL_FREE(temp);
                                }
                                else
                                    TRACELOG(
                                        LOG_WARNING,
                                        "MODEL: [%s] Color attribute data format not supported",
                                        fileName);
                            }
                            else
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Color attribute data format not supported",
                                    fileName);
                        }

                        // NOTE: Attributes related to animations are processed separately
                    }

                    // Load primitive indices data (if provided)
                    if (mesh->primitives[p].indices != nullptr)
                    {
                        cgltf_accessor* attribute = mesh->primitives[p].indices;

                        model.meshes[meshIndex].triangleCount = (int)attribute->count / 3;

                        if (attribute->component_type == cgltf_component_type_r_16u)
                        {
                            // Init raylib mesh indices to copy glTF attribute data
                            model.meshes[meshIndex].indices =
                                (unsigned short*)RL_MALLOC(attribute->count * sizeof(unsigned short));

                            // Load unsigned short data type into mesh.indices
                            LOAD_ATTRIBUTE(attribute, 1, unsigned short, model.meshes[meshIndex].indices)
                        }
                        else if (attribute->component_type == cgltf_component_type_r_8u)
                        {
                            // Init raylib mesh indices to copy glTF attribute data
                            model.meshes[meshIndex].indices =
                                (unsigned short*)RL_MALLOC(attribute->count * sizeof(unsigned short));
                            LOAD_ATTRIBUTE_CAST(
                                attribute, 1, unsigned char, model.meshes[meshIndex].indices, unsigned short)
                        }
                        else if (attribute->component_type == cgltf_component_type_r_32u)
                        {
                            // Init raylib mesh indices to copy glTF attribute data
                            model.meshes[meshIndex].indices =
                                (unsigned short*)RL_MALLOC(attribute->count * sizeof(unsigned short));
                            LOAD_ATTRIBUTE_CAST(
                                attribute, 1, unsigned int, model.meshes[meshIndex].indices, unsigned short);

                            TRACELOG(
                                LOG_WARNING,
                                "MODEL: [%s] Indices data converted from u32 to u16, possible loss of data",
                                fileName);
                        }
                        else
                        {
                            TRACELOG(
                                LOG_WARNING, "MODEL: [%s] Indices data format not supported, use u16", fileName);
                        }
                    }
                    else
                        model.meshes[meshIndex].triangleCount =
                            model.meshes[meshIndex].vertexCount / 3; // Unindexed mesh

                    // Assign to the primitive mesh the corresponding material index
                    // NOTE: If no material defined, mesh uses the already assigned default material (index: 0)
                    for (unsigned int m = 0; m < data->materials_count; m++)
                    {
                        // The primitive actually keeps the pointer to the corresponding material,
                        // raylib instead assigns to the mesh the by its index, as loaded in model.materials
                        // array To get the index, we check if material pointers match, and we assign the
                        // corresponding index, skipping index 0, the default material
                        if (&data->materials[m] == mesh->primitives[p].material)
                        {
                            model.meshMaterial[meshIndex] = m + 1;
                            break;
                        }
                    }

                    meshIndex++; // Move to next mesh
                }
            }

            // Load glTF meshes animation data
            // REF: https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#skins
            // REF: https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#skinned-mesh-attributes
            //
            // LIMITATIONS:
            //  - Only supports 1 armature per file, and skips loading it if there are multiple armatures
            //  - Only supports linear interpolation (default method in Blender when checked "Always Sample
            //  Animations" when exporting a GLTF file)
            //  - Only supports translation/rotation/scale animation channel.path, weights not considered (i.e.
            //  morph targets)
            //----------------------------------------------------------------------------------------------------
            if (data->skins_count > 0)
            {
                cgltf_skin skin = data->skins[0];
                model.bones = sgLoadBoneInfoGLTF(skin, &model.boneCount);
                model.bindPose = (Transform*)RL_MALLOC(model.boneCount * sizeof(Transform));

                for (int i = 0; i < model.boneCount; i++)
                {
                    cgltf_node* node = skin.joints[i];
                    cgltf_float worldTransform[16];
                    cgltf_node_transform_world(node, worldTransform);
                    Matrix worldMatrix = {
                        worldTransform[0],
                        worldTransform[4],
                        worldTransform[8],
                        worldTransform[12],
                        worldTransform[1],
                        worldTransform[5],
                        worldTransform[9],
                        worldTransform[13],
                        worldTransform[2],
                        worldTransform[6],
                        worldTransform[10],
                        worldTransform[14],
                        worldTransform[3],
                        worldTransform[7],
                        worldTransform[11],
                        worldTransform[15]};
                    MatrixDecompose(
                        worldMatrix,
                        &(model.bindPose[i].translation),
                        &(model.bindPose[i].rotation),
                        &(model.bindPose[i].scale));
                }
            }
            if (data->skins_count > 1)
            {
                TRACELOG(
                    LOG_WARNING,
                    "MODEL: [%s] can only load one skin (armature) per model, but gltf skins_count == %i",
                    fileName,
                    data->skins_count);
            }

            meshIndex = 0;
            for (unsigned int i = 0; i < data->nodes_count; i++)
            {
                cgltf_node* node = &(data->nodes[i]);

                cgltf_mesh* mesh = node->mesh;
                if (!mesh) continue;

                for (unsigned int p = 0; p < mesh->primitives_count; p++)
                {
                    // NOTE: We only support primitives defined by triangles
                    if (mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;

                    for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
                    {
                        // NOTE: JOINTS_1 + WEIGHT_1 will be used for +4 joints influencing a vertex -> Not
                        // supported by raylib

                        if (mesh->primitives[p].attributes[j].type ==
                            cgltf_attribute_type_joints) // JOINTS_n (vec4: 4 bones max per vertex / u8, u16)
                        {
                            cgltf_accessor* attribute = mesh->primitives[p].attributes[j].data;

                            // NOTE: JOINTS_n can only be vec4 and u8/u16
                            // SPECS: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes-overview

                            // WARNING: raylib only supports model.meshes[].boneIds as u8 (unsigned char),
                            // if data is provided in any other format, it is converted to supported format but
                            // it could imply data loss (a warning message is issued in that case)

                            if (attribute->type == cgltf_type_vec4)
                            {
                                if (attribute->component_type == cgltf_component_type_r_8u)
                                {
                                    // Init raylib mesh boneIds to copy glTF attribute data
                                    model.meshes[meshIndex].boneIds = (unsigned char*)RL_CALLOC(
                                        model.meshes[meshIndex].vertexCount * 4, sizeof(unsigned char));

                                    // Load attribute: vec4, u8 (unsigned char)
                                    LOAD_ATTRIBUTE(attribute, 4, unsigned char, model.meshes[meshIndex].boneIds)
                                }
                                else if (attribute->component_type == cgltf_component_type_r_16u)
                                {
                                    // Init raylib mesh boneIds to copy glTF attribute data
                                    model.meshes[meshIndex].boneIds = (unsigned char*)RL_CALLOC(
                                        model.meshes[meshIndex].vertexCount * 4, sizeof(unsigned char));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto* temp = (unsigned short*)RL_CALLOC(
                                        model.meshes[meshIndex].vertexCount * 4, sizeof(unsigned short));
                                    LOAD_ATTRIBUTE(attribute, 4, unsigned short, temp);

                                    // Convert data to raylib color data type (4 bytes)
                                    bool boneIdOverflowWarning = false;
                                    for (int b = 0; b < model.meshes[meshIndex].vertexCount * 4; b++)
                                    {
                                        if ((temp[b] > 255) && !boneIdOverflowWarning)
                                        {
                                            TRACELOG(
                                                LOG_WARNING,
                                                "MODEL: [%s] Joint attribute data format (u16) overflow",
                                                fileName);
                                            boneIdOverflowWarning = true;
                                        }

                                        // Despite the possible overflow, we convert data to unsigned char
                                        model.meshes[meshIndex].boneIds[b] = (unsigned char)temp[b];
                                    }

                                    RL_FREE(temp);
                                }
                                else
                                    TRACELOG(
                                        LOG_WARNING,
                                        "MODEL: [%s] Joint attribute data format not supported",
                                        fileName);
                            }
                            else
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Joint attribute data format not supported",
                                    fileName);
                        }
                        else if (
                            mesh->primitives[p].attributes[j].type ==
                            cgltf_attribute_type_weights) // WEIGHTS_n (vec4, u8n/u16n/f32)
                        {
                            cgltf_accessor* attribute = mesh->primitives[p].attributes[j].data;

                            if (attribute->type == cgltf_type_vec4)
                            {
                                // TODO: Support component types: u8, u16?
                                if (attribute->component_type == cgltf_component_type_r_8u)
                                {
                                    // Init raylib mesh bone weight to copy glTF attribute data
                                    model.meshes[meshIndex].boneWeights =
                                        (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount * 4, sizeof(float));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto temp =
                                        (unsigned char*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned char));
                                    LOAD_ATTRIBUTE(attribute, 4, unsigned char, temp);

                                    // Convert data to raylib bone weight data type (4 bytes)
                                    for (unsigned int b = 0; b < attribute->count * 4; b++)
                                        model.meshes[meshIndex].boneWeights[b] = (float)temp[b] / 255.0f;

                                    RL_FREE(temp);
                                }
                                else if (attribute->component_type == cgltf_component_type_r_16u)
                                {
                                    // Init raylib mesh bone weight to copy glTF attribute data
                                    model.meshes[meshIndex].boneWeights =
                                        (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount * 4, sizeof(float));

                                    // Load data into a temp buffer to be converted to raylib data type
                                    auto temp =
                                        (unsigned short*)RL_MALLOC(attribute->count * 4 * sizeof(unsigned short));
                                    LOAD_ATTRIBUTE(attribute, 4, unsigned short, temp);

                                    // Convert data to raylib bone weight data type
                                    for (unsigned int b = 0; b < attribute->count * 4; b++)
                                        model.meshes[meshIndex].boneWeights[b] = (float)temp[b] / 65535.0f;

                                    RL_FREE(temp);
                                }
                                else if (attribute->component_type == cgltf_component_type_r_32f)
                                {
                                    // Init raylib mesh bone weight to copy glTF attribute data
                                    model.meshes[meshIndex].boneWeights =
                                        (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount * 4, sizeof(float));

                                    // Load 4 components of float data type into mesh.boneWeights
                                    // for cgltf_attribute_type_weights we have:
                                    //   - data.meshes[0] (256 vertices)
                                    //   - 256 values, provided as cgltf_type_vec4 of float (4 byte per joint,
                                    //   stride 16)
                                    LOAD_ATTRIBUTE(attribute, 4, float, model.meshes[meshIndex].boneWeights)
                                }
                                else
                                    TRACELOG(
                                        LOG_WARNING,
                                        "MODEL: [%s] Joint weight attribute data format not supported, use "
                                        "vec4 "
                                        "float",
                                        fileName);
                            }
                            else
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Joint weight attribute data format not supported, use vec4 "
                                    "float",
                                    fileName);
                        }
                    }

                    // Animated vertex data
                    model.meshes[meshIndex].animVertices =
                        (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount * 3, sizeof(float));
                    memcpy(
                        model.meshes[meshIndex].animVertices,
                        model.meshes[meshIndex].vertices,
                        model.meshes[meshIndex].vertexCount * 3 * sizeof(float));
                    model.meshes[meshIndex].animNormals =
                        (float*)RL_CALLOC(model.meshes[meshIndex].vertexCount * 3, sizeof(float));
                    if (model.meshes[meshIndex].normals != nullptr)
                    {
                        memcpy(
                            model.meshes[meshIndex].animNormals,
                            model.meshes[meshIndex].normals,
                            model.meshes[meshIndex].vertexCount * 3 * sizeof(float));
                    }

                    // Bone Transform Matrices
                    model.meshes[meshIndex].boneCount = model.boneCount;
                    model.meshes[meshIndex].boneMatrices =
                        (Matrix*)RL_CALLOC(model.meshes[meshIndex].boneCount, sizeof(Matrix));

                    for (int j = 0; j < model.meshes[meshIndex].boneCount; j++)
                    {
                        model.meshes[meshIndex].boneMatrices[j] = MatrixIdentity();
                    }

                    meshIndex++; // Move to next mesh
                }
            }

            // Free all cgltf loaded data
            cgltf_free(data);
        }
        else
            TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);

        // WARNING: cgltf requires the file pointer available while reading data
        UnloadFileData(fileData);

        return model;
    }

    // Get interpolated pose for bone sampler at a specific time. Returns true on success
    static bool sgGetPoseAtTimeGLTF(
        cgltf_interpolation_type interpolationType,
        cgltf_accessor* input,
        cgltf_accessor* output,
        float time,
        void* data)
    {
        if (interpolationType >= cgltf_interpolation_type_max_enum) return false;

        // Input and output should have the same count
        float tstart = 0.0f;
        float tend = 0.0f;
        int keyframe = 0; // Defaults to first pose

        for (int i = 0; i < (int)input->count - 1; i++)
        {
            cgltf_bool r1 = cgltf_accessor_read_float(input, i, &tstart, 1);
            if (!r1) return false;

            cgltf_bool r2 = cgltf_accessor_read_float(input, i + 1, &tend, 1);
            if (!r2) return false;

            if ((tstart <= time) && (time < tend))
            {
                keyframe = i;
                break;
            }
        }

        // Constant animation, no need to interpolate
        if (FloatEquals(tend, tstart)) return true;

        float duration = fmaxf((tend - tstart), EPSILON);
        float t = (time - tstart) / duration;
        t = (t < 0.0f) ? 0.0f : t;
        t = (t > 1.0f) ? 1.0f : t;

        if (output->component_type != cgltf_component_type_r_32f) return false;

        if (output->type == cgltf_type_vec3)
        {
            switch (interpolationType)
            {
            case cgltf_interpolation_type_step: {
                float tmp[3] = {0.0f};
                cgltf_accessor_read_float(output, keyframe, tmp, 3);
                Vector3 v1 = {tmp[0], tmp[1], tmp[2]};
                auto r = (Vector3*)data;

                *r = v1;
            }
            break;
            case cgltf_interpolation_type_linear: {
                float tmp[3] = {0.0f};
                cgltf_accessor_read_float(output, keyframe, tmp, 3);
                Vector3 v1 = {tmp[0], tmp[1], tmp[2]};
                cgltf_accessor_read_float(output, keyframe + 1, tmp, 3);
                Vector3 v2 = {tmp[0], tmp[1], tmp[2]};
                auto r = (Vector3*)data;

                *r = Vector3Lerp(v1, v2, t);
            }
            break;
            case cgltf_interpolation_type_cubic_spline: {
                float tmp[3] = {0.0f};
                cgltf_accessor_read_float(output, 3 * keyframe + 1, tmp, 3);
                Vector3 v1 = {tmp[0], tmp[1], tmp[2]};
                cgltf_accessor_read_float(output, 3 * keyframe + 2, tmp, 3);
                Vector3 tangent1 = {tmp[0], tmp[1], tmp[2]};
                cgltf_accessor_read_float(output, 3 * (keyframe + 1) + 1, tmp, 3);
                Vector3 v2 = {tmp[0], tmp[1], tmp[2]};
                cgltf_accessor_read_float(output, 3 * (keyframe + 1), tmp, 3);
                Vector3 tangent2 = {tmp[0], tmp[1], tmp[2]};
                auto r = (Vector3*)data;

                *r = Vector3CubicHermite(v1, tangent1, v2, tangent2, t);
            }
            break;
            default:
                break;
            }
        }
        else if (output->type == cgltf_type_vec4)
        {
            // Only v4 is for rotations, so we know it's a quaternion
            switch (interpolationType)
            {
            case cgltf_interpolation_type_step: {
                float tmp[4] = {0.0f};
                cgltf_accessor_read_float(output, keyframe, tmp, 4);
                Vector4 v1 = {tmp[0], tmp[1], tmp[2], tmp[3]};
                auto r = (Vector4*)data;

                *r = v1;
            }
            break;
            case cgltf_interpolation_type_linear: {
                float tmp[4] = {0.0f};
                cgltf_accessor_read_float(output, keyframe, tmp, 4);
                Vector4 v1 = {tmp[0], tmp[1], tmp[2], tmp[3]};
                cgltf_accessor_read_float(output, keyframe + 1, tmp, 4);
                Vector4 v2 = {tmp[0], tmp[1], tmp[2], tmp[3]};
                auto r = (Vector4*)data;

                *r = QuaternionSlerp(v1, v2, t);
            }
            break;
            case cgltf_interpolation_type_cubic_spline: {
                float tmp[4] = {0.0f};
                cgltf_accessor_read_float(output, 3 * keyframe + 1, tmp, 4);
                Vector4 v1 = {tmp[0], tmp[1], tmp[2], tmp[3]};
                cgltf_accessor_read_float(output, 3 * keyframe + 2, tmp, 4);
                Vector4 outTangent1 = {tmp[0], tmp[1], tmp[2], 0.0f};
                cgltf_accessor_read_float(output, 3 * (keyframe + 1) + 1, tmp, 4);
                Vector4 v2 = {tmp[0], tmp[1], tmp[2], tmp[3]};
                cgltf_accessor_read_float(output, 3 * (keyframe + 1), tmp, 4);
                Vector4 inTangent2 = {tmp[0], tmp[1], tmp[2], 0.0f};
                auto r = (Vector4*)data;

                v1 = QuaternionNormalize(v1);
                v2 = QuaternionNormalize(v2);

                if (Vector4DotProduct(v1, v2) < 0.0f)
                {
                    v2 = Vector4Negate(v2);
                }

                outTangent1 = Vector4Scale(outTangent1, duration);
                inTangent2 = Vector4Scale(inTangent2, duration);

                *r = QuaternionCubicHermiteSpline(v1, outTangent1, v2, inTangent2, t);
            }
            break;
            default:
                break;
            }
        }

        return true;
    }

#define GLTF_ANIMDELAY 17 // Animation frames delay, (~1000 ms/60 FPS = 16.666666* ms)

    static ModelAnimation* sgLoadModelAnimationsGLTF(const char* fileName, int* animCount)
    {
        // glTF file loading
        int dataSize = 0;
        unsigned char* fileData = LoadFileData(fileName, &dataSize);

        ModelAnimation* animations = nullptr;

        // glTF data loading
        cgltf_options options{};
        options.file.read = sgLoadFileGLTFCallback;
        options.file.release = sgReleaseFileGLTFCallback;
        cgltf_data* data = nullptr;
        cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

        if (result != cgltf_result_success)
        {
            TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);
            *animCount = 0;
            return nullptr;
        }

        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success)
            TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load animation buffers", fileName);

        if (result == cgltf_result_success)
        {
            if (data->skins_count > 0)
            {
                cgltf_skin skin = data->skins[0];
                *animCount = (int)data->animations_count;
                animations = (ModelAnimation*)RL_MALLOC(data->animations_count * sizeof(ModelAnimation));

                for (unsigned int i = 0; i < data->animations_count; i++)
                {
                    animations[i].bones = sgLoadBoneInfoGLTF(skin, &animations[i].boneCount);

                    cgltf_animation animData = data->animations[i];

                    struct Channels
                    {
                        cgltf_animation_channel* translate;
                        cgltf_animation_channel* rotate;
                        cgltf_animation_channel* scale;
                        cgltf_interpolation_type interpolationType;
                    };

                    auto boneChannels = (Channels*)RL_CALLOC(animations[i].boneCount, sizeof(struct Channels));
                    float animDuration = 0.0f;

                    for (unsigned int j = 0; j < animData.channels_count; j++)
                    {
                        cgltf_animation_channel channel = animData.channels[j];
                        int boneIndex = -1;

                        for (unsigned int k = 0; k < skin.joints_count; k++)
                        {
                            if (animData.channels[j].target_node == skin.joints[k])
                            {
                                boneIndex = k;
                                break;
                            }
                        }

                        if (boneIndex == -1)
                        {
                            // Animation channel for a node not in the armature
                            continue;
                        }

                        boneChannels[boneIndex].interpolationType = animData.channels[j].sampler->interpolation;

                        if (animData.channels[j].sampler->interpolation != cgltf_interpolation_type_max_enum)
                        {
                            if (channel.target_path == cgltf_animation_path_type_translation)
                            {
                                boneChannels[boneIndex].translate = &animData.channels[j];
                            }
                            else if (channel.target_path == cgltf_animation_path_type_rotation)
                            {
                                boneChannels[boneIndex].rotate = &animData.channels[j];
                            }
                            else if (channel.target_path == cgltf_animation_path_type_scale)
                            {
                                boneChannels[boneIndex].scale = &animData.channels[j];
                            }
                            else
                            {
                                TRACELOG(
                                    LOG_WARNING,
                                    "MODEL: [%s] Unsupported target_path on channel %d's sampler for "
                                    "animation "
                                    "%d. Skipping.",
                                    fileName,
                                    j,
                                    i);
                            }
                        }
                        else
                            TRACELOG(
                                LOG_WARNING,
                                "MODEL: [%s] Invalid interpolation curve encountered for GLTF animation.",
                                fileName);

                        float t = 0.0f;
                        cgltf_bool r = cgltf_accessor_read_float(
                            channel.sampler->input, channel.sampler->input->count - 1, &t, 1);

                        if (!r)
                        {
                            TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load input time", fileName);
                            continue;
                        }

                        animDuration = (t > animDuration) ? t : animDuration;
                    }

                    if (animData.name != nullptr)
                    {
                        strncpy(animations[i].name, animData.name, sizeof(animations[i].name));
                        animations[i].name[sizeof(animations[i].name) - 1] = '\0';
                    }

                    animations[i].frameCount = (int)(animDuration * 1000.0f / GLTF_ANIMDELAY) + 1;
                    animations[i].framePoses =
                        (Transform**)RL_MALLOC(animations[i].frameCount * sizeof(Transform*));

                    for (int j = 0; j < animations[i].frameCount; j++)
                    {
                        animations[i].framePoses[j] =
                            (Transform*)RL_MALLOC(animations[i].boneCount * sizeof(Transform));
                        float time = ((float)j * GLTF_ANIMDELAY) / 1000.0f;

                        for (int k = 0; k < animations[i].boneCount; k++)
                        {
                            Vector3 translation = {
                                skin.joints[k]->translation[0],
                                skin.joints[k]->translation[1],
                                skin.joints[k]->translation[2]};
                            Quaternion rotation = {
                                skin.joints[k]->rotation[0],
                                skin.joints[k]->rotation[1],
                                skin.joints[k]->rotation[2],
                                skin.joints[k]->rotation[3]};
                            Vector3 scale = {
                                skin.joints[k]->scale[0], skin.joints[k]->scale[1], skin.joints[k]->scale[2]};

                            if (boneChannels[k].translate)
                            {
                                if (!sgGetPoseAtTimeGLTF(
                                        boneChannels[k].interpolationType,
                                        boneChannels[k].translate->sampler->input,
                                        boneChannels[k].translate->sampler->output,
                                        time,
                                        &translation))
                                {
                                    TRACELOG(
                                        LOG_INFO,
                                        "MODEL: [%s] Failed to load translate pose data for bone %s",
                                        fileName,
                                        animations[i].bones[k].name);
                                }
                            }

                            if (boneChannels[k].rotate)
                            {
                                if (!sgGetPoseAtTimeGLTF(
                                        boneChannels[k].interpolationType,
                                        boneChannels[k].rotate->sampler->input,
                                        boneChannels[k].rotate->sampler->output,
                                        time,
                                        &rotation))
                                {
                                    TRACELOG(
                                        LOG_INFO,
                                        "MODEL: [%s] Failed to load rotate pose data for bone %s",
                                        fileName,
                                        animations[i].bones[k].name);
                                }
                            }

                            if (boneChannels[k].scale)
                            {
                                if (!sgGetPoseAtTimeGLTF(
                                        boneChannels[k].interpolationType,
                                        boneChannels[k].scale->sampler->input,
                                        boneChannels[k].scale->sampler->output,
                                        time,
                                        &scale))
                                {
                                    TRACELOG(
                                        LOG_INFO,
                                        "MODEL: [%s] Failed to load scale pose data for bone %s",
                                        fileName,
                                        animations[i].bones[k].name);
                                }
                            }

                            animations[i].framePoses[j][k] =
                                Transform{.translation = translation, .rotation = rotation, .scale = scale};
                        }

                        sgBuildPoseFromParentJoints(
                            animations[i].bones, animations[i].boneCount, animations[i].framePoses[j]);
                    }

                    TRACELOG(
                        LOG_INFO,
                        "MODEL: [%s] Loaded animation: %s (%d frames, %fs)",
                        fileName,
                        (animData.name != nullptr) ? animData.name : "nullptr",
                        animations[i].frameCount,
                        animDuration);
                    RL_FREE(boneChannels);
                }
            }

            if (data->skins_count > 1)
            {
                TRACELOG(
                    LOG_WARNING,
                    "MODEL: [%s] expected exactly one skin to load animation data from, but found %i",
                    fileName,
                    data->skins_count);
            }

            cgltf_free(data);
        }
        UnloadFileData(fileData);
        return animations;
    }
#endif

#endif // SUPPORT_MODULE_RMODELS
} // sage