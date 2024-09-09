#include "MapLoader.hpp"

#include "components/Collideable.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
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

    void createFloor(entt::registry* registry, BoundingBox bb)
    {
        entt::entity floor = registry->create();
        auto& floorCollidable = registry->emplace<Collideable>(floor, bb, MatrixIdentity());
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

        // TODO: Need a better tagging system for the meshes.
        // TODO: Fairly sure I could map this in a json file or something
        if (meshName.find("SM_Bld") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::BUILDING;
        }
        else if (meshName.find("SM_Env_Mountain") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::TERRAIN;
        }
        else if (meshName.find("SM_Env") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::FLOOR;
            floorMeshes.push_back(&collideable);
        }
        else if (meshName.find("SM_Prop") != std::string::npos)
        {
            collideable.collisionLayer = CollisionLayer::BUILDING;
        }
        else
        {
            collideable.collisionLayer = CollisionLayer::DEFAULT;
        }
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
            std::string materialName = "DEFAULT"; // Load default raylib mat

            if (IsFileExtension(filePath.c_str(), ".obj"))
            {
                size_t lastindex = filePath.find_last_of(".");
                std::string mtlPath = fileName.substr(0, lastindex) + ".mtl";
                if (FileExists(mtlPath.c_str()))
                {
                    std::ifstream infile(filePath);
                    materialName = "";
                    std::string line;
                    while (std::getline(infile, line))
                    {
                        if (line.substr(0, 6) == "map_Kd")
                        {
                            infile.close();
                            // Find the position of the first non-whitespace character after "map_Kd"
                            size_t textureNameStart = line.find_first_not_of(" \t", 6);

                            if (textureNameStart != std::string::npos)
                            {
                                // Return the substring from the first non-whitespace character to the end
                                materialName = line.substr(textureNameStart);
                            }
                        }
                    }
                }

                ResourceManager::GetInstance().EmplaceModel(fileName, materialName, filePath);
            }
        }
        std::cout << "FINISH: Loading mesh data into resource manager. \n";

        std::cout << "START: Processing txt data into resource manager. \n";
        for (const auto& entry : fs::directory_iterator(path)) // txt files
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().stem().string();
            if (IsFileExtension(filePath.c_str(), ".txt"))
            {
                processTxtFile(registry, meshPath, entry.path());
            }
        }
        std::cout << "FINISH: Processing txt data into resource manager. \n";

        // Calculate grid based on walkable area
        // TODO: Below should be based on the bounding box of all the floor meshes, as opposed to a magic number.
        BoundingBox mapBB{Vector3{-500, 0, -500}, Vector3{500, 0, 500}}; // min, max
        // BoundingBox mapBB = calculateFloorSize(floorMeshes);

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

        // Height map gets saved here.
        serializer::SaveMap(*registry, heightmap, normalMap);

        CloseWindow();
        std::cout << "Map saved." << std::endl;
    }
}; // namespace sage