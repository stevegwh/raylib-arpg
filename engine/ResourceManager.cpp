//
// Created by Steve Wheeler on 16/07/2024.
//

#include "ResourceManager.hpp"

#include "components/Renderable.hpp"

#include "raylib/src/config.h"
#include "raymath.h"
#include "rlgl.h"
#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_NONE

#include "external/cgltf.h"

#include <ranges>
extern "C"
{
#include "raylib/src/external/tinyobj_loader_c.h"
}
#include <stb_include.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>

namespace sage
{
    namespace
    {
        constexpr const char* DefaultMaterialName = "Default";

        std::string FallbackMaterialName(const std::string& sourcePath, int materialIndex)
        {
            return sourcePath + "#Material" + std::to_string(materialIndex);
        }

        std::string Trim(const std::string& value)
        {
            const size_t begin = value.find_first_not_of(" \t\r\n");
            if (begin == std::string::npos) return "";

            const size_t end = value.find_last_not_of(" \t\r\n");
            return value.substr(begin, end - begin + 1);
        }

        bool IsAbsolutePath(const std::string& path)
        {
            return !path.empty() && (path[0] == '/' || path[0] == '\\' || (path.size() > 1 && path[1] == ':'));
        }

        std::string ResolveObjMaterialPath(const char* objFileName, const std::string& materialFileName)
        {
            if (IsAbsolutePath(materialFileName)) return materialFileName;

            std::string directory = GetDirectoryPath(objFileName);
            if (directory.empty()) return materialFileName;

            if (directory.back() != '/' && directory.back() != '\\')
            {
                directory += '/';
            }

            return directory + materialFileName;
        }

        std::string UnnamedMaterialName(const char* fileName, size_t materialIndex)
        {
            return std::string(fileName) + "#Material" + std::to_string(materialIndex);
        }

        std::string FindObjMaterialLibrary(const char* fileName)
        {
            std::ifstream objFile(fileName);
            if (!objFile.is_open()) return "";

            std::string materialFileName;
            std::string line;
            while (std::getline(objFile, line))
            {
                const std::string trimmed = Trim(line);
                if (trimmed.empty() || trimmed[0] == '#') continue;

                constexpr const char* keyword = "mtllib";
                constexpr size_t keywordLength = 6;
                if (trimmed.compare(0, keywordLength, keyword) != 0) continue;
                if (trimmed.size() == keywordLength ||
                    (trimmed[keywordLength] != ' ' && trimmed[keywordLength] != '\t'))
                {
                    continue;
                }

                // raylib's tinyobj C loader keeps the last mtllib declaration it sees.
                materialFileName = Trim(trimmed.substr(keywordLength));
            }

            return materialFileName;
        }

        std::vector<std::string> LoadObjMaterialNames(const char* fileName)
        {
            const std::string materialFileName = FindObjMaterialLibrary(fileName);
            if (materialFileName.empty()) return {};

            tinyobj_material_t* materials = nullptr;
            unsigned int materialCount = 0;
            const std::string materialPath = ResolveObjMaterialPath(fileName, materialFileName);
            const int result = tinyobj_parse_mtl_file(&materials, &materialCount, materialPath.c_str());
            if (result != TINYOBJ_SUCCESS)
            {
                TraceLog(
                    LOG_WARNING,
                    "MODEL: [%s] Failed to parse OBJ material file: %s",
                    fileName,
                    materialPath.c_str());
                return {};
            }

            std::vector<std::string> names;
            names.reserve(materialCount);
            for (unsigned int i = 0; i < materialCount; ++i)
            {
                const char* name = materials[i].name;
                names.emplace_back((name != nullptr && name[0] != '\0') ? name : UnnamedMaterialName(fileName, i));
            }

            tinyobj_materials_free(materials, materialCount);
            return names;
        }

        struct FileData
        {
            unsigned char* bytes = nullptr;

            ~FileData()
            {
                if (bytes != nullptr) UnloadFileData(bytes);
            }
        };

        struct GltfData
        {
            cgltf_data* data = nullptr;

            ~GltfData()
            {
                if (data != nullptr) cgltf_free(data);
            }
        };

        std::vector<std::string> LoadGltfMaterialNames(const char* fileName)
        {
            int dataSize = 0;
            FileData fileData{LoadFileData(fileName, &dataSize)};
            if (fileData.bytes == nullptr) return {};

            cgltf_options options{};
            GltfData gltf;
            const cgltf_result result = cgltf_parse(&options, fileData.bytes, dataSize, &gltf.data);
            if (result != cgltf_result_success)
            {
                TraceLog(LOG_WARNING, "MODEL: [%s] Failed to parse glTF material names", fileName);
                return {};
            }

            std::vector<std::string> names;
            names.reserve(gltf.data->materials_count + 1);
            names.emplace_back(DefaultMaterialName);

            // raylib's glTF loader reserves material slot 0 for its default material.
            for (size_t i = 0; i < gltf.data->materials_count; ++i)
            {
                const char* name = gltf.data->materials[i].name;
                names.emplace_back(
                    (name != nullptr && name[0] != '\0') ? name : UnnamedMaterialName(fileName, i + 1));
            }

            return names;
        }

        std::vector<std::string> LoadMaterialNames(const char* fileName)
        {
            if (fileName == nullptr || fileName[0] == '\0') return {};

            if (IsFileExtension(fileName, ".obj"))
            {
                return LoadObjMaterialNames(fileName);
            }

            if (IsFileExtension(fileName, ".gltf") || IsFileExtension(fileName, ".glb"))
            {
                return LoadGltfMaterialNames(fileName);
            }

            return {};
        }

        void NormalizeMaterialNames(
            Model& model, std::vector<std::string>& materialNames, const std::string& sourcePath)
        {
            if (model.materialCount <= 0)
            {
                materialNames.clear();
                return;
            }

            if (static_cast<int>(materialNames.size()) > model.materialCount)
            {
                materialNames.resize(model.materialCount);
            }

            const size_t originalSize = materialNames.size();
            materialNames.resize(model.materialCount);

            for (int i = 0; i < model.materialCount; ++i)
            {
                if (!materialNames[i].empty()) continue;

                // With no extractor data, a single raylib material is the default material.
                // With old glTF-packed data, slot 0 may be empty because raylib reserves it.
                if ((originalSize == 0 && model.materialCount == 1) || (originalSize > 0 && i == 0))
                {
                    materialNames[i] = DefaultMaterialName;
                }
                else
                {
                    materialNames[i] = FallbackMaterialName(sourcePath, i);
                }
            }
        }
    } // namespace

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

    // Replaces a freshly-loaded model's materials with shared singletons in materialMap.
    // raylib-allocated materials that get displaced are released via UnloadMaterial.
    // Materials whose names are not yet pooled are donated to materialMap (first-write wins).
    void ResourceManager::dedupeAndShareMaterials(
        Model& model, std::vector<std::string>& materialNames, const std::string& sourcePath)
    {
        NormalizeMaterialNames(model, materialNames, sourcePath);

        for (int i = 0; i < model.materialCount; ++i)
        {
            const auto& name = materialNames[i];
            if (!materialMap.contains(name))
            {
                // First sighting of this name: donate the freshly-loaded material to the shared pool.
                materialMap[name] = model.materials[i];
            }
            else
            {
                // Already pooled: release raylib's freshly-allocated copy, swap in the shared one.
                UnloadMaterial(model.materials[i]);
                model.materials[i] = materialMap.at(name);
            }
        }
    }

    Music ResourceManager::GetMusic(const std::string& path)
    {
        auto key = StripPath(path); // Will either be a mesh alias (MDL_GOBLIN) or a mesh name (e.g., QUEST_BONE
                                    // from QUEST_BONE.obj)
        if (!music.contains(key))
        {
            music[key] = LoadMusicStream(path.c_str());
        }
        return music.at(key);
    }

    Sound ResourceManager::GetSFX(const std::string& path)
    {
        auto key = StripPath(path); // Will either be a mesh alias (MDL_GOBLIN) or a mesh name (e.g., QUEST_BONE
                                    // from QUEST_BONE.obj)
        if (!sfx.contains(key))
        {
            // NB: Currently, the resource packer does not support serializing sound/music.
            sfx[key] = LoadSound(path.c_str());
        }
        return sfx.at(key);
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
        auto key = StripPath(path); // Will either be a mesh alias (MDL_GOBLIN) or a mesh name (e.g., QUEST_BONE
        //  from QUEST_BONE.obj)
        if (!nonModelTextures.contains(key))
        {
            if (!images.contains(key))
            {
                images.emplace(key, LoadImage(path.c_str()));
            }
            nonModelTextures[key] = LoadTextureFromImage(images[key]);
        }
        return nonModelTextures[key];
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
        // assert(fonts.contains(path));
        if (!fonts.contains(path))
        {
            FontLoadFromFile(path);
        }
        return fonts[path];
    }

    void ResourceManager::ImageUnload(const std::string& key)
    {
        if (images.contains(key))
        {
            UnloadImage(images.at(key));
            images.erase(key);
        }
    }

    ImageSafe ResourceManager::GetImage(const std::string& key)
    {
        assert(images.contains(key));
        return ImageSafe(images[key], false);
    }

    void ResourceManager::FontLoadFromFile(const std::string& path)
    {
        assert(FileExists(path.c_str()));
        if (!fonts.contains(path))
        {
            auto font = LoadFont(path.c_str());
            for (size_t i = 0; i < font.glyphCount; i++)
            {
                assert(font.glyphs[i].image.data != nullptr);
            }

            fonts[path] = font;
        }
    }

    void ResourceManager::ImageLoadFromFile(const std::string& path)
    {
        auto key = StripPath(path); // Will either be a mesh alias (MDL_GOBLIN) or a mesh name (e.g., QUEST_BONE
        // from QUEST_BONE.obj)
        assert(FileExists(path.c_str()));
        // As the path is stripped of any relative information, file name overlap is very possible.
        assert(!images.contains(key));
        images[key] = LoadImage(path.c_str());
    }

    void ResourceManager::ImageLoadFromFile(const std::string& path, Image image)
    {
        auto key = StripPath(path); // Will either be a mesh alias (MDL_GOBLIN) or a mesh name (e.g., QUEST_BONE
        // from QUEST_BONE.obj)
        assert(!images.contains(key));
        images[key] = image;
        image = {};
    }

    void ResourceManager::ModelLoadFromFile(const std::string& path)
    {
        auto key = StripPath(path); // Will either be a mesh alias (MDL_GOBLIN) or a mesh name (e.g., QUEST_BONE
                                    // from QUEST_BONE.obj)
        ModelLoadFromFile(path, key);
    }

    void ResourceManager::ModelLoadFromFile(const std::string& path, const std::string& key)
    {
        assert(!key.empty());
        if (modelCopies.contains(key)) return;
        assert(FileExists(path.c_str()));

        auto materialNames = LoadMaterialNames(path.c_str());
        Model model = LoadModel(path.c_str());
        dedupeAndShareMaterials(model, materialNames, path);
        modelCopies.emplace(key, ModelInfo{model, std::move(materialNames), path});
    }

    void ResourceManager::StoreModel(const ModelInfo& modelInfo, const std::string& key)
    {
        modelCopies.emplace(key, modelInfo);
    }

    namespace
    {
        // Registry of generators for primitives baked into the asset pack. CreateModelMutable
        // uses these to regenerate a fresh mesh when the source asset has no on-disk file.
        // Hardcoded list — raylib's GenMesh* functions, parameterized at canonical unit sizes
        // (callers scale via the entity's transform).
        using PrimitiveGenerator = Mesh (*)();

        Mesh GenSphere()
        {
            return GenMeshSphere(1.0f, 32, 32);
        }
        Mesh GenHemiSphere()
        {
            return GenMeshHemiSphere(1.0f, 16, 32);
        }
        Mesh GenPlane()
        {
            return GenMeshPlane(1.0f, 1.0f, 1, 1);
        }
        Mesh GenCube()
        {
            return GenMeshCube(1.0f, 1.0f, 1.0f);
        }
        Mesh GenCylinder()
        {
            return GenMeshCylinder(1.0f, 1.0f, 32);
        }
        Mesh GenCone()
        {
            return GenMeshCone(1.0f, 1.0f, 32);
        }
        Mesh GenTorus()
        {
            return GenMeshTorus(0.25f, 1.0f, 16, 32);
        }
        Mesh GenKnot()
        {
            return GenMeshKnot(1.0f, 2.0f, 16, 128);
        }
        Mesh GenPoly()
        {
            return GenMeshPoly(5, 1.0f);
        }

        const std::unordered_map<std::string, PrimitiveGenerator>& PrimitiveGenerators()
        {
            static const std::unordered_map<std::string, PrimitiveGenerator> generators = {
                {"primitive_sphere", &GenSphere},
                {"primitive_hemisphere", &GenHemiSphere},
                {"primitive_plane", &GenPlane},
                {"primitive_cube", &GenCube},
                {"primitive_cylinder", &GenCylinder},
                {"primitive_cone", &GenCone},
                {"primitive_torus", &GenTorus},
                {"primitive_knot", &GenKnot},
                {"primitive_poly", &GenPoly},
            };
            return generators;
        }
    } // namespace

    /* Non-owning view onto the shared model entry stored under viewKey. Read-only API.
    The returned ModelView's lifetime is independent of RM: it just borrows; the
    underlying entry stays alive until UnloadAll (i.e. scene tear-down). */
    std::vector<std::string> ResourceManager::GetModelKeys(const bool includeGenerated) const
    {
        std::vector<std::string> keys;
        keys.reserve(modelCopies.size());

        for (const auto& [key, info] : modelCopies)
        {
            if (key.find("#mut_") != std::string::npos) continue;
            if (!includeGenerated && info.sourcePath.empty()) continue;
            keys.push_back(key);
        }

        std::sort(keys.begin(), keys.end());
        return keys;
    }

    ModelView ResourceManager::GetModelView(const std::string& viewKey) const
    {
        assert(modelCopies.contains(viewKey));
        ModelView view;
        view.rlmodel = modelCopies.at(viewKey).model;
        view.assetKey = viewKey;
        return view;
    }

    /* Create a new deep-copy entry in the mutable pool from the asset stored under
    viewKey, and returns a ModelMutable view onto it. The deep copy has private
    materials, so mutations through the returned view are isolated. Lifetime of
    the new entry is scene-tied (released at UnloadAll). For models loaded from
    disk this re-loads via sourcePath; for baked primitives it regenerates via a
    registered generator function. */
    ModelMutable ResourceManager::CreateModelMutable(const std::string& viewKey)
    {
        assert(modelCopies.contains(viewKey));
        const auto& info = modelCopies.at(viewKey);

        const std::string instanceKey = viewKey + "#mut_" + std::to_string(mutableInstanceCounter++);
        assert(!modelCopies.contains(instanceKey) && "CreateModelMutable: instanceKey collision");

        Model model;
        const auto& generators = PrimitiveGenerators();
        if (!info.sourcePath.empty())
        {
            // On-disk asset — re-load to get private mesh + material allocations.
            assert(FileExists(info.sourcePath.c_str()) && "CreateModelMutable: source file missing at runtime");
            model = LoadModel(info.sourcePath.c_str());
        }
        else if (const auto it = generators.find(viewKey); it != generators.end())
        {
            // Baked primitive — regenerate mesh, raylib allocates fresh default materials.
            model = LoadModelFromMesh(it->second());
        }
        else
        {
            assert(false && "CreateModelMutable: asset has no sourcePath and is not a registered primitive");
            return {};
        }

        modelCopies.emplace(
            instanceKey, ModelInfo{model, info.materialNames, info.sourcePath, /*privateMaterials=*/true});

        ModelMutable mut;
        mut.rlmodel = modelCopies.at(instanceKey).model;
        mut.assetKey = viewKey;
        mut.instanceKey = instanceKey;
        return mut;
    }

    void ResourceManager::ModelAnimationLoadFromFile(const std::string& path)
    {
        auto key = StripPath(path); // Will either be a mesh alias (MDL_GOBLIN) or a mesh name (e.g., QUEST_BONE
        // from QUEST_BONE.obj)
        if (!modelAnimations.contains(key))
        {
            int animsCount;
            auto animations = LoadModelAnimations(path.c_str(), &animsCount);
            if (animations == nullptr)
            {
                std::cout << "ResourceManager: Model does not contain animation data, or was unable to be loaded. "
                             "Aborting... \n";
                return;
            }
            modelAnimations[key] = std::make_pair(animations, animsCount);
        }
    }

    ModelAnimation* ResourceManager::GetModelAnimation(const std::string& key, int* animsCount) const
    {
        if (!modelAnimations.contains(key))
        {
            TraceLog(
                LOG_FATAL, "ResourceManager::GetModelAnimation: animation '%s' was not pre-loaded.", key.c_str());
            assert(false && "missing model animation");
        }
        const auto& pair = modelAnimations.at(key);
        *animsCount = pair.second;
        return pair.first;
    }

    void ResourceManager::UnloadImages()
    {
        for (const auto& image : images | std::views::values)
        {
            UnloadImage(image);
        }
        images.clear();
    }

    void ResourceManager::UnloadShaderFileText()
    {
        for (const auto& vs : vertShaderFileText | std::views::values)
        {
            UnloadFileText(vs);
        }
        for (const auto& fs : fragShaderFileText | std::views::values)
        {
            UnloadFileText(fs);
        }
        vertShaderFileText.clear();
        fragShaderFileText.clear();
    }

    void sgUnloadModel(const Model& model)
    {
        // Unload meshes
        for (int i = 0; i < model.meshCount; i++)
            UnloadMesh(model.meshes[i]);

        // Unload arrays
        RL_FREE(model.meshes);
        RL_FREE(model.materials);
        RL_FREE(model.meshMaterial);

        // Unload animation data
        RL_FREE(model.bones);
        RL_FREE(model.bindPose);

        TRACELOG(LOG_INFO, "MODEL: Unloaded model (and meshes) from RAM and VRAM");
    }

    void ResourceManager::UnloadAll()
    {
        for (auto& s : sfx | std::views::values)
        {
            UnloadSound(s);
        }
        for (auto& mus : music | std::views::values)
        {
            UnloadMusicStream(mus);
        }
        for (auto& [key, mat] : materialMap)
        {
            for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
            {
                if (mat.maps[i].texture.id != rlGetTextureIdDefault()) rlUnloadTexture(mat.maps[i].texture.id);
            }
            std::cout << "Material key: " << key << std::endl;
            std::cout << "Material maps address : " << &mat.maps << std::endl;
            RL_FREE(mat.maps);
        }
        std::cout << "Unloading models" << std::endl;
        for (auto& info : modelCopies | std::views::values)
        {
            if (info.privateMaterials)
            {
                // Deep-copy entry: free the per-material maps allocations, but don't
                // UnloadMaterial — textures and shaders inside are RM-cached and still
                // in use by other entries.
                Model& m = info.model;
                for (int i = 0; i < m.materialCount; ++i)
                {
                    RL_FREE(m.materials[i].maps);
                }
                for (int i = 0; i < m.meshCount; ++i)
                {
                    UnloadMesh(m.meshes[i]);
                }
                RL_FREE(m.meshes);
                RL_FREE(m.materials);
                RL_FREE(m.meshMaterial);
                RL_FREE(m.bones);
                RL_FREE(m.bindPose);
            }
            else
            {
                sgUnloadModel(info.model);
            }
        }
        for (const auto& tex : nonModelTextures | std::views::values)
        {
            UnloadTexture(tex);
        }
        for (const auto& image : images | std::views::values)
        {
            UnloadImage(image);
        }
        for (const auto& [fst, snd] : modelAnimations | std::views::values)
        {
            UnloadModelAnimations(fst, snd);
        }
        for (const auto& shader : shaders | std::views::values)
        {
            UnloadShader(shader);
        }
        for (const auto& text : vertShaderFileText | std::views::values)
        {
            UnloadFileText(text);
        }
        for (const auto& text : fragShaderFileText | std::views::values)
        {
            UnloadFileText(text);
        }
        for (const auto& font : fonts | std::views::values)
        {
            UnloadFont(font);
        }
        fonts.clear();
        shaders.clear();
        materialMap.clear();
        images.clear();
        nonModelTextures.clear();
        modelCopies.clear();
        modelAnimations.clear();
        vertShaderFileText.clear();
        fragShaderFileText.clear();
        music.clear();
        sfx.clear();
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
