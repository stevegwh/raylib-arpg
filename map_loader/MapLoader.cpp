#include "MapLoader.hpp"

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

    void parseMtlFile(const std::string& mtlPath, std::string& materialName)
    {
        if (!FileExists(mtlPath.c_str())) return;
        std::ifstream infile(mtlPath.c_str());
        std::string line;
        while (std::getline(infile, line))
        {
            if (line.substr(0, 6) != "newmtl") continue;

            infile.close();
            size_t textureNameStart = line.find_first_not_of(" \t", 6);

            if (textureNameStart != std::string::npos)
            {
                materialName = line.substr(textureNameStart);
            }
            else
            {
                std::cout << "ERROR: Failed to read material name \n";
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

        // Strip file extension (Could do this in blender script, instead).
        size_t lastindex = meshName.find_last_of(".");
        meshName = meshName.substr(0, lastindex);

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

        auto model = ResourceManager::GetInstance().LoadModelCopy(meshName);
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

    void MapLoader::ConstructMap(
        entt::registry* registry, NavigationGridSystem* navigationGridSystem, const char* path)
    {
        auto meshPath = std::string(std::string(path) + "/mesh");
        if (!DirectoryExists(path) || !DirectoryExists(meshPath.c_str()))
        {
            std::cout << "ERROR: MapLoader -> Directory does not exist or path invalid." << std::endl;
            exit(1);
        }

        InitWindow(300, 100, "Loading Map!");

        std::cout << "START: Loading mesh data into resource manager. \n";
        for (const auto& entry : fs::directory_iterator(meshPath))
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().stem().string();
            std::cout << filePath << std::endl;

            if (IsFileExtension(filePath.c_str(), ".obj"))
            {
                std::string materialName = "DEFAULT"; // Load default raylib mat
                size_t lastindex = filePath.find_last_of(".");
                std::string mtlPath = filePath.substr(0, lastindex) + ".mtl";
                parseMtlFile(mtlPath, materialName);
                ResourceManager::GetInstance().EmplaceModel(fileName, materialName, filePath);
            }
        }
        std::cout << "FINISH: Loading mesh data into resource manager. \n";

        std::vector<Collideable*> floorMeshes;

        std::cout << "START: Processing txt data into resource manager. \n";
        for (const auto& entry : fs::directory_iterator(path)) // txt files
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
        ImageSafe heightmap, normalMap;
        navigationGridSystem->Init(500, 1.0f);
        navigationGridSystem->InitGridHeightNormals(); // Calculates grid terrain height and gets normals
        navigationGridSystem->GenerateHeightMap(heightmap);
        navigationGridSystem->GenerateNormalMap(normalMap);

        // Exporting for debug purposes
        // ExportImage(heightmap.GetImage(), "heightmap.png");
        // ExportImage(normalMap.GetImage(), "normalmap.png");

        // TODO: Need a library of enums to modelKeys and modelKey/paths and store them all in a separate bin.
        // "assetlib.bin" or something. For now, we can just hand-load them into the map binary.
        // TODO: We need to be able to serialize animation data
        // ResourceManager::GetInstance().EmplaceModel("resources/models/gltf/arissa.glb");
        ResourceManager::GetInstance().EmplaceModel("resources/models/gltf/arissa.glb");
        int tmp;
        ResourceManager::GetInstance().ModelAnimationLoad("resources/models/gltf/arissa.glb", &tmp);
        // ResourceManager::GetInstance().EmplaceModel("resources/models/gltf/hero2.glb");
        ResourceManager::GetInstance().EmplaceModel("resources/models/obj/portal.obj");

        // Height map gets saved here.
        serializer::SaveMap(*registry, heightmap, normalMap);

        CloseWindow();
        std::cout << "Map saved." << std::endl;
    }
}; // namespace sage