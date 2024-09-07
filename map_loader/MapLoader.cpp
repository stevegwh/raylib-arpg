#include "MapLoader.hpp"

#include "components/Collideable.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
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

    void createFloor(entt::registry* registry, BoundingBox bb)
    {
        entt::entity floor = registry->create();
        auto& floorCollidable = registry->emplace<Collideable>(floor, bb);
        floorCollidable.collisionLayer = CollisionLayer::FLOOR;
    }

    Vector3 scaleFromOrigin(const Vector3& point, float scale)
    {
        return Vector3Scale(point, scale);
    }

    void processTxtFile(
        entt::registry* registry, const std::string& meshPath, const std::filesystem::path& txtPath)
    {
        std::vector<Collideable*> floorMeshes;

        // Temporary
        MaterialPaths matPaths{};
        matPaths.diffuse = "resources/models/obj/PolyAdventureTexture_01.png";
        // ---

        std::ifstream infile(txtPath);
        std::string key;

        infile >> key;
        std::string objectName;
        infile >> objectName;

        infile >> key;
        std::string meshName;
        infile >> meshName;

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
        std::cout << meshPath + meshName << std::endl;

        auto model = ResourceManager::GetInstance().LoadModelCopy(meshPath + "/" + meshName);

        auto& renderable =
            registry->emplace<Renderable>(entity, std::move(model), matPaths, MatrixScale(scalex, scaley, scalez));
        renderable.name = objectName;

        auto& trans = registry->emplace<sgTransform>(entity, entity);
        Vector3 scaledPosition = scaleFromOrigin({x, y, z}, WORLD_SCALE);
        trans.SetPosition({scaledPosition.x, scaledPosition.y, scaledPosition.z});
        trans.SetRotation({rotx * RAD2DEG, roty * RAD2DEG, rotz * RAD2DEG});
        trans.SetScale(
            {trans.GetScale().x * WORLD_SCALE,
             trans.GetScale().y * WORLD_SCALE,
             trans.GetScale().z * WORLD_SCALE});

        // Almost works
        auto bb = renderable.GetModel()->CalcLocalBoundingBox();
        bb.min = Vector3Transform(bb.min, MatrixScale(trans.GetScale().x, trans.GetScale().y, trans.GetScale().z));
        bb.max = Vector3Transform(bb.max, MatrixScale(trans.GetScale().x, trans.GetScale().y, trans.GetScale().z));
        bb.min = Vector3Transform(bb.min, MatrixTranslate(scaledPosition.x, scaledPosition.y, scaledPosition.z));
        bb.max = Vector3Transform(bb.max, MatrixTranslate(scaledPosition.x, scaledPosition.y, scaledPosition.z));

        auto& collideable = registry->emplace<Collideable>(entity, bb);

        if (renderable.name.find("SM_Bld") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::BUILDING;
        }
        else if (renderable.name.find("SM_Env_NoWalk") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::TERRAIN;
        }
        else if (renderable.name.find("SM_Env") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::FLOOR;
            floorMeshes.push_back(&collideable);
        }
        else if (renderable.name.find("SM_Prop") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::BUILDING;
        }
        else
        {
            collideable.collisionLayer = CollisionLayer::DEFAULT;
        }
    }

    void MapLoader::ConstructMap(entt::registry* registry, const char* path)
    {
        auto meshPath = std::string(std::string(path) + "/mesh");
        if (!DirectoryExists(path) || !DirectoryExists(meshPath.c_str()))
        {
            std::cout << "ERROR: MapLoader -> Directory does not exist or path invalid." << std::endl;
            exit(1);
        }

        InitWindow(100, 100, "Loading Map!");

        for (const auto& entry : fs::directory_iterator(meshPath))
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().stem().string();
            std::cout << filePath << std::endl;
            if (IsFileExtension(filePath.c_str(), ".obj"))
            {
                ResourceManager::GetInstance().EmplaceModel(filePath);
            }
        }

        for (const auto& entry : fs::directory_iterator(path)) // txt files
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().stem().string();
            if (IsFileExtension(filePath.c_str(), ".txt"))
            {
                processTxtFile(registry, meshPath, entry.path());
            }
        }

        // Calculate grid based on walkable area
        BoundingBox mapBB{Vector3{-500, 0, -500}, Vector3{500, 0, 500}}; // min, max
        // BoundingBox mapBB = calculateFloorSize(floorMeshes);

        // Create floor
        createFloor(registry, mapBB);

        serializer::Save(*registry);

        // Generate height/normal maps here.
        CloseWindow();
        std::cout << "Map saved." << std::endl;
    }
}; // namespace sage