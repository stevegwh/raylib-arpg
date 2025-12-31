//
// Created by Steve Wheeler on 26/12/2024.
//

/**
 * This (WIP) file is a modified version of rmodels.c. The original license is included below.
 *
 * *********************************************************************************************
 *
 *   rmodels - Basic functions to draw 3d shapes and load and draw 3d models
 *   LICENSE: zlib/libpng
 *
 *   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
 *
 *   This software is provided "as-is", without any express or implied warranty. In no event
 *   will the authors be held liable for any damages arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose, including commercial
 *   applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not claim that you
 *     wrote the original software. If you use this software in a product, an acknowledgment
 *     in the product documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *     as being the original software.
 *
 *     3. This notice may not be removed or altered from any source distribution.
 *
 **********************************************************************************************/

#include "Slibmodel.hpp"

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" // Defines module configuration flags
#endif

#include "raylib.h"
#include "raymath.h" // Required for: Vector3, Quaternion and Matrix functionality
#include "rlgl.h"    // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2
#include "utils.h"   // Required for: TRACELOG(), LoadFileData(), LoadFileText(), SaveFileText()

#include <cassert>
#include <cstdlib> // Required for: malloc(), calloc(), free()
#include <cstring> // Required for: memcmp(), strlen(), strncpy()

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define CGLTF_MALLOC RL_MALLOC
#define CGLTF_FREE RL_FREE
#include "external/cgltf.h" // glTF file format loading

namespace sage
{
    static std::vector<std::string> sgLoadOBJ(const char* fileName);  // Load OBJ mesh data
    static std::vector<std::string> sgLoadGLTF(const char* fileName); // Load GLTF mesh data

    // Load model from files (mesh and material)
    // TODO: Horrible function made out of desperation to extract the material names from the files.
    std::vector<std::string> GetMaterialNames(const char* fileName)
    {
        std::vector<std::string> matNames{};

        if (IsFileExtension(fileName, ".obj"))
        {
            matNames = sgLoadOBJ(fileName);
        }
        else if (IsFileExtension(fileName, ".gltf") || IsFileExtension(fileName, ".glb"))
        {
            matNames = sgLoadGLTF(fileName);
        }

        return matNames;
    }

    static std::vector<std::string> sgLoadOBJ(const char* fileName)
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
        if (!ret) return {};

        std::vector<std::string> matNames;
        matNames.reserve(materials.size());
        for (const auto& mat : materials)
        {
            std::cout << "Material name: " << mat.name << std::endl;
            matNames.push_back(mat.name);
        }

        return matNames;
    }

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

    // Load glTF file into model struct, .gltf and .glb supported
    static std::vector<std::string> sgLoadGLTF(const char* fileName)
    {

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

        // glTF file loading
        int dataSize = 0;
        unsigned char* fileData = LoadFileData(fileName, &dataSize);

        if (fileData == nullptr) return {};

        // glTF data loading
        cgltf_options options{};
        options.file.read = sgLoadFileGLTFCallback;
        options.file.release = sgReleaseFileGLTFCallback;
        cgltf_data* data = nullptr;
        cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

        if (result == cgltf_result_success)
        {
            std::vector<std::string> matNames;
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

            // LoadGLTF reserves index 0 for the default material, so we need to offset the names here
            matNames.resize(data->materials_count + 1);
            for (int i = 1; i <= data->materials_count; ++i)
            {
                matNames[i] = data->materials[i - 1].name;
            }
            return matNames;
        }
        else
        {
            return {};
        }
    }
} // namespace sage