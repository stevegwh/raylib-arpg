#include "ResourcePacker.hpp"

#include "AssetManager.hpp"
#include "components/Collideable.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "GameObjectFactory.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include <ResourceManager.hpp>
#include <Serializer.hpp>

#include "raylib.h"
#include "raymath.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

namespace sage
{

    constexpr float WORLD_SCALE = 5.0f;

    CollisionLayer setCollisionLayer(const std::string& meshName)
    {
        // TODO: Need a better tagging system for the meshes.
        if (meshName.find("SM_Bld") != std::string::npos)
        {
            return CollisionLayer::BUILDING;
        }
        if (meshName.find("SM_Env_Mountain") != std::string::npos)
        {
            return CollisionLayer::TERRAIN;
        }
        if (meshName.find("SM_Env") != std::string::npos)
        {
            return CollisionLayer::FLOOR;
        }
        if (meshName.find("SM_Prop") != std::string::npos)
        {
            return CollisionLayer::BUILDING;
        }

        return CollisionLayer::DEFAULT;
    }

    void parseMtlFile(const std::string& mtlPath, std::string& materialKey)
    {
        if (!FileExists(mtlPath.c_str()))
        {
            std::cout << "WARNING: MTL file could not be found: " << mtlPath << std::endl;
            return;
        }
        std::ifstream infile(mtlPath.c_str());
        std::string line;
        while (std::getline(infile, line))
        {
            if (line.substr(0, 6) != "newmtl") continue;

            infile.close();
            size_t textureNameStart = line.find_first_not_of(" \t", 6);

            if (textureNameStart != std::string::npos)
            {
                materialKey = line.substr(textureNameStart);
            }
            else
            {
                std::cout << "ERROR: Failed to read material name \n";
                exit(1);
            }
        }
    }

    void createFloor(entt::registry* registry, BoundingBox bb)
    {
        entt::entity floor = registry->create();
        auto& floorCollidable = registry->emplace<Collideable>(floor, bb, MatrixIdentity());
        floorCollidable.collisionLayer = CollisionLayer::FLOOR;
    }

    BoundingBox calculateFloorSize(const std::vector<Collideable*>& floorMeshes)
    {
        // TODO: Below doesn't seem to work always, depending on the map.
        BoundingBox mapBB{Vector3{0, 0, 0}, Vector3{0, 0, 0}}; // min, max
        for (const auto& col : floorMeshes)
        {
            if (col->worldBoundingBox.min.x <= mapBB.min.x && col->worldBoundingBox.min.z <= mapBB.min.z)
            {
                mapBB.min = col->worldBoundingBox.min;
            }
            if (col->worldBoundingBox.max.x >= mapBB.max.x && col->worldBoundingBox.max.z >= mapBB.max.z)
            {
                mapBB.max = col->worldBoundingBox.max;
            }
        }
        mapBB.min.y = 0.1f;
        mapBB.max.y = 0.1f;
        return mapBB;
    }

    Vector3 scaleFromOrigin(const Vector3& point, float scale)
    {
        return Vector3Scale(point, scale);
    }

    void processTxtFile(
        entt::registry* registry,
        const std::string& meshPath,
        const std::filesystem::path& txtPath,
        std::vector<Collideable*>& floorMeshes)
    {

        std::ifstream infile(txtPath);
        std::string key;

        infile >> key;
        std::string objectName;
        infile >> objectName;

        infile >> key;
        std::string meshName;
        infile >> meshName;
        meshName = meshPath + "/" + meshName; // Full path is used as key

        infile >> key;
        float x, y, z;
        infile >> x >> y >> z;

        infile >> key;
        float rotx, roty, rotz;
        infile >> rotx >> roty >> rotz;

        infile >> key;
        float scalex, scaley, scalez;
        infile >> scalex >> scaley >> scalez;

        auto entity = registry->create();

        auto model = ResourceManager::GetInstance().GetModelCopy(meshName);
        assert(!meshName.empty());
        model.SetKey(meshName);

        // You could just use x,y,z and the regular scale and then later store "scaled position" and "scalex *
        // WORLD_SCALE" into the sgTransform data. This makes the mesh centre its position in world space, as
        // opposed to the world centre. For static objects, this won't really matter.
        Vector3 scaledPosition = scaleFromOrigin({x, y, z}, WORLD_SCALE);
        Matrix rotMat =
            MatrixMultiply(MatrixMultiply(MatrixRotateZ(rotz), MatrixRotateY(roty)), MatrixRotateX(rotx));
        Matrix transMat = MatrixTranslate(scaledPosition.x, scaledPosition.y, scaledPosition.z);
        Matrix scaleMat = MatrixScale(scalex * WORLD_SCALE, scaley * WORLD_SCALE, scalez * WORLD_SCALE);

        Matrix mat = MatrixMultiply(MatrixMultiply(scaleMat, rotMat), transMat);

        auto& renderable = registry->emplace<Renderable>(entity, std::move(model), mat);
        renderable.name = objectName;

        auto& trans = registry->emplace<sgTransform>(entity, entity);

        auto& collideable = registry->emplace<Collideable>(
            entity, renderable.GetModel()->CalcLocalBoundingBox(), trans.GetMatrix());

        collideable.collisionLayer = setCollisionLayer(meshName);
        if (collideable.collisionLayer == CollisionLayer::FLOOR) floorMeshes.push_back(&collideable);
    }

    void ResourcePacker::ConstructMap(
        entt::registry* registry,
        NavigationGridSystem* navigationGridSystem,
        const char* input,
        const char* output)
    {
        registry->clear();
        ResourceManager::GetInstance().Reset();

        auto meshPath = std::string(std::string(input) + "/mesh");
        if (!DirectoryExists(input) || !DirectoryExists(meshPath.c_str()))
        {
            std::cout << "ERROR: MapLoader -> Directory does not exist or path invalid." << std::endl;
            exit(1);
        }

        InitWindow(300, 100, "Loading Map!");

        std::cout << "START: Constructing map into bin file. \n";

        std::cout << "START: Loading mesh data into resource manager. \n";
        for (const auto& entry : fs::directory_iterator(meshPath))
        {
            std::string filePath = entry.path().string();
            // std::string fileName = entry.path().stem().string();
            std::cout << filePath << std::endl;

            if (IsFileExtension(filePath.c_str(), ".obj"))
            {
                std::string materialKey = "DEFAULT"; // Load default raylib mat
                size_t lastindex = filePath.find_last_of('.');
                std::string mtlPath = filePath.substr(0, lastindex) + ".mtl";
                parseMtlFile(mtlPath, materialKey);
                ResourceManager::GetInstance().ModelLoadFromFile(filePath, materialKey);
            }
        }
        std::cout << "FINISH: Loading mesh data into resource manager. \n";

        std::vector<Collideable*> floorMeshes;

        std::cout << "START: Processing txt data into resource manager. \n";
        for (const auto& entry : fs::directory_iterator(input)) // txt files
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().stem().string();
            if (IsFileExtension(filePath.c_str(), ".txt"))
            {
                processTxtFile(registry, meshPath, entry.path(), floorMeshes);
            }
        }
        std::cout << "FINISH: Processing txt data into resource manager. \n";

        // Calculate grid based on walkable area
        // BoundingBox mapBB{Vector3{-500, 0, -500}, Vector3{500, 0, 500}}; // min, max
        BoundingBox mapBB = calculateFloorSize(floorMeshes);
        // Create floor
        createFloor(registry, mapBB);

        // Generate height/normal maps here.
        ImageSafe heightMap(false), normalMap(false);
        navigationGridSystem->Init(500, 1.0f);
        navigationGridSystem->InitGridHeightNormals(); // Calculates grid terrain height and gets normals
        navigationGridSystem->GenerateHeightMap(heightMap);
        navigationGridSystem->GenerateNormalMap(normalMap);

        // Uncomment to export for debug purposes
        // ExportImage(heightmap.GetImage(), "heightmap.png");
        // ExportImage(normalMap.GetImage(), "normalmap.png");

        ResourceManager::GetInstance().ImageLoadFromFile("HEIGHT_MAP", heightMap.GetImage());
        ResourceManager::GetInstance().ImageLoadFromFile("NORMAL_MAP", normalMap.GetImage());
        // Images are nulled at this point

        serializer::SaveMap(*registry, output);
        std::cout << "FINISH: Constructing map into bin file. \n";
    }

    /**
     * output: The path + filename of the resulting binary
     **/
    void ResourcePacker::PackAssets(entt::registry* registry, const std::string& output)
    {
        registry->clear();
        ResourceManager::GetInstance().Reset();

        std::filesystem::path fullpath(output);
        if (!DirectoryExists(fullpath.remove_filename().c_str()))
        {
            std::cout << "ResourcePacker: Directory does not exist, cannot save. Aborting... \n";
            return;
        }
        if (!IsFileExtension(output.c_str(), ".bin"))
        {
            std::cout << "ResourcePacker: File extension for output file must be 'bin'. Aborting... \n";
            return;
        }

        registry->clear();
        ResourceManager::GetInstance().Reset();

        // We pack all resources used by the game here.
        // These are *all* loaded into memory for the moment.
        AssetManager::GetInstance().LoadPaths();

        std::cout << "START: Loading assets into memory \n";

        for (int i = 0; i < magic_enum::enum_underlying(AssetID::COUNT); ++i)
        {
            auto id = magic_enum::enum_cast<AssetID>(i).value();
            auto name = std::string(magic_enum::enum_name(id));
            auto tag = name.substr(0, 3);

            if (tag == "IMG")
            {
                ResourceManager::GetInstance().ImageLoadFromFile(id);
            }
            else if (tag == "MDL")
            {
                ResourceManager::GetInstance().ModelLoadFromFile(id);

                if (IsFileExtension(AssetManager::GetInstance().GetAssetPath(id).c_str(), ".glb") ||
                    IsFileExtension(AssetManager::GetInstance().GetAssetPath(id).c_str(), ".gltf"))
                {
                    ResourceManager::GetInstance().ModelAnimationLoadFromFile(id);
                }
            }
            // else if (std::find(name.begin(), name.end(), "TXT") != name.end())
            // {
            //     // Load text here.
            // }
        }
        std::cout << "FINISH: Loading assets into memory \n";
        serializer::SaveCurrentResourceData(*registry, output.c_str());
    }
}; // namespace sage