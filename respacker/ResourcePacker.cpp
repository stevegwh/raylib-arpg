#include "ResourcePacker.hpp"

#include "engine/components/Collideable.hpp"
#include "engine/components/DoorBehaviorComponent.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/Spawner.hpp"
#include "engine/Light.hpp"
#include "engine/LightManager.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Serializer.hpp"
#include "engine/slib.hpp"
#include "engine/systems/CollisionSystem.hpp"
#include "engine/systems/NavigationGridSystem.hpp"

#include "game/src/components/QuestComponents.hpp"
#include "game/src/GameObjectFactory.hpp"
#include "game/src/ItemFactory.hpp"
#include "game/src/QuestManager.hpp"
#include "game/utils/Serializer.hpp"

#include "raylib.h"
#include "raymath.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

namespace sage
{

    constexpr float WORLD_SCALE = 5.0f;

    Vector3 scaleFromOrigin(const Vector3& point, float scale)
    {
        return Vector3Scale(point, scale);
    }

    CollisionLayer getCollisionLayer(const std::string& objectName)
    {
        if (objectName.find("_BLD_") != std::string::npos)
        {
            return CollisionLayer::BUILDING;
        }
        if (objectName.find("_WALL_") != std::string::npos)
        {
            return CollisionLayer::BUILDING;
        }
        if (objectName.find("_HOLE_") != std::string::npos)
        {
            return CollisionLayer::BUILDING;
        }
        if (objectName.find("_BG_") != std::string::npos)
        {
            return CollisionLayer::BACKGROUND;
        }
        if (objectName.find("_FLOORSIMPLE_") != std::string::npos)
        {
            // Uses bounding box bounds for height (flat surfaces).
            return CollisionLayer::GEOMETRY_SIMPLE;
        }
        if (objectName.find("_FLOORCOMPLEX_") != std::string::npos)
        {
            // Samples mesh for height/normal information
            return CollisionLayer::GEOMETRY_COMPLEX;
        }
        if (objectName.find("_PROP_") != std::string::npos)
        {
            return CollisionLayer::BUILDING;
        }
        if (objectName.find("_STAIRS_") != std::string::npos)
        {
            return CollisionLayer::STAIRS;
        }
        if (objectName.find("_MAPBASE_") != std::string::npos)
        {
            return CollisionLayer::BACKGROUND;
        }
        if (objectName.find("_ITEM_") != std::string::npos)
        {
            return CollisionLayer::ITEM;
        }
        if (objectName.find("_DOOR_") != std::string::npos)
        {
            return CollisionLayer::BUILDING;
        }
        if (objectName.find("_INTERACTABLE_") != std::string::npos)
        {
            return CollisionLayer::INTERACTABLE;
        }
        if (objectName.find("_CHEST_") != std::string::npos)
        {
            return CollisionLayer::CHEST;
        }

        return CollisionLayer::BACKGROUND; // by default, objects are ignored
    }

    std::string readLine(std::ifstream& infile, const std::string& key)
    {
        std::string line;
        std::getline(infile, line);
        if (line.substr(0, key.length()) != key)
        {
            throw std::runtime_error("Expected key '" + key + "' not found");
        }

        // Remove carriage returns from the line
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        return line.substr(key.length() + 2); // +2 to skip ": "
    }

    void HandleLight(entt::registry* registry, std::ifstream& infile)
    {
        std::string light_type;
        std::string objectName;
        float x, y, z;
        int r, g, b, s;
        try
        {
            light_type = readLine(infile, "light_type");

            objectName = readLine(infile, "name");

            std::istringstream locStream(readLine(infile, "location"));
            locStream >> x >> y >> z;

            std::istringstream rgbStream(readLine(infile, "color"));
            rgbStream >> r >> g >> b;

            std::istringstream strStream(readLine(infile, "strength"));
            strStream >> s;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error parsing spawner data: " << e.what() << std::endl;
            assert(0);
        }

        auto entity = registry->create();
        auto& light = registry->emplace<Light>(entity);
        light.target = Vector3Zero();

        if (light_type == "sun")
        {
            light.type = LIGHT_DIRECTIONAL;
            light.position = {0, 1500, 0};
            light.brightness = 0.75; // TODO
        }
        else if (light_type == "point")
        {
            light.type = LIGHT_POINT;
            light.position = scaleFromOrigin({x, y, z}, WORLD_SCALE);
            light.brightness = s / 150; // Seems to work well.
        }

        light.color =
            Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), 1};
    }

    void HandleSpawner(entt::registry* registry, std::ifstream& infile)
    {
        std::string objectName;
        std::string spawnerType;
        std::string spawnerName;
        float x, y, z, rotx, roty, rotz;
        try
        {
            objectName = readLine(infile, "name");

            std::istringstream locStream(readLine(infile, "location"));
            locStream >> x >> y >> z;

            std::istringstream rotStream(readLine(infile, "rotation"));
            rotStream >> rotx >> roty >> rotz;

            spawnerType = readLine(infile, "spawner_type");
            spawnerName = readLine(infile, "spawner_name");
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error parsing spawner data: " << e.what() << std::endl;
            assert(0);
        }

        auto entity = registry->create();

        Vector3 scaledPosition = scaleFromOrigin({x, y, z}, WORLD_SCALE);
        auto& spawner = registry->emplace<Spawner>(entity);
        spawner.pos = {scaledPosition.x, scaledPosition.y, scaledPosition.z};
        spawner.rot = {rotx * RAD2DEG, roty * RAD2DEG, rotz * RAD2DEG};

        std::unordered_map<std::string, SpawnerType> spawnerMap{
            {"ENEMY", SpawnerType::ENEMY},
            {"PLAYER", SpawnerType::PLAYER},
            {"NPC", SpawnerType::NPC},
            {"DIALOG_CUTSCENE", SpawnerType::DIALOG_CUTSCENE}};

        spawner.type = spawnerMap.at(spawnerType);
        spawner.name = spawnerName;
    }

    entt::entity HandleMesh(entt::registry* registry, std::ifstream& infile, const fs::path& meshPath, int& slices)
    {
        std::string meshName, objectName;
        float x, y, z, rotx, roty, rotz, scalex, scaley, scalez;
        try
        {
            objectName = readLine(infile, "name");
            meshName = StripPath(readLine(infile, "mesh"));

            std::istringstream locStream(readLine(infile, "location"));
            locStream >> x >> y >> z;

            std::istringstream rotStream(readLine(infile, "rotation"));
            rotStream >> rotx >> roty >> rotz;

            std::istringstream scaleStream(readLine(infile, "scale"));
            scaleStream >> scalex >> scaley >> scalez;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error parsing mesh data: " << e.what() << std::endl;
            assert(0);
        }

        auto entity = registry->create();

        auto model = ResourceManager::GetInstance().GetModelCopy(meshName);
        model.SetKey(meshName);

        Vector3 scaledPosition = scaleFromOrigin({x, y, z}, WORLD_SCALE);
        Matrix rotMat =
            MatrixMultiply(MatrixMultiply(MatrixRotateZ(rotz), MatrixRotateY(roty)), MatrixRotateX(rotx));
        Matrix transMat = MatrixTranslate(scaledPosition.x, scaledPosition.y, scaledPosition.z);
        Matrix scaleMat = MatrixScale(scalex * WORLD_SCALE, scaley * WORLD_SCALE, scalez * WORLD_SCALE);
        Matrix mat = MatrixMultiply(MatrixMultiply(scaleMat, rotMat), transMat);

        auto& renderable =
            registry->emplace<Renderable>(entity, std::move(model), MatrixMultiply(scaleMat, rotMat));
        renderable.SetName(objectName);

        // We set the transform component's position instead of setting the model's transform.
        // By doing this, we can track where certain (static) objects are.
        auto& trans = registry->emplace<sgTransform>(entity, entity);
        trans.SetPosition(scaledPosition);

        auto& collideable = registry->emplace<Collideable>(
            entity, renderable.GetModel()->CalcLocalBoundingBox(), trans.GetMatrix());

        collideable.collisionLayer = getCollisionLayer(objectName);

        if (objectName.find("_MAPBASE_") != std::string::npos)
        {
            slices = std::ceil(
                std::max(
                    collideable.worldBoundingBox.max.x - collideable.worldBoundingBox.min.x,
                    collideable.worldBoundingBox.max.z - collideable.worldBoundingBox.min.z));
            if (slices % 2 == 1)
            {
                slices += 1;
            }
        }

        return entity;
    }

    void HandleItem(
        entt::registry* registry, lq::ItemFactory* itemFactory, std::ifstream& infile, const fs::path& meshPath)
    {
        int x;
        auto itemEntity = HandleMesh(registry, infile, meshPath, x);
        auto itemName = registry->get<Renderable>(itemEntity).GetModel()->GetKey();
        itemFactory->AttachItem(itemEntity, itemName);
    }

    void processTxtFile(
        entt::registry* registry,
        lq::ItemFactory* itemFactory,
        const fs::path& meshPath,
        const fs::path& txtPath,
        int& slices)
    {
        std::string typeName;
        std::ifstream infile(txtPath);

        try
        {
            typeName = readLine(infile, "type");
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error parsing file " << txtPath << ": " << e.what() << std::endl;
            assert(0);
        }

        if (typeName.find("spawner") != std::string::npos)
        {
            HandleSpawner(registry, infile);
        }
        else if (typeName.find("light") != std::string::npos)
        {
            HandleLight(registry, infile);
        }
        else if (typeName.find("item") != std::string::npos)
        {
            HandleItem(registry, itemFactory, infile, meshPath);
        }
        else
        {
            HandleMesh(registry, infile, meshPath, slices);
        }
    }

    void ResourcePacker::ConstructMap(
        entt::registry* registry,
        NavigationGridSystem* navigationGridSystem,
        const char* input,
        const char* output)
    {
        registry->clear();
        ResourceManager::GetInstance().Reset();
        lq::ItemFactory itemFactory{registry};
        serializer::DeserializeJsonFile<lq::ItemFactory>("resources/items.json", itemFactory);

        fs::path inputPath(input);
        fs::path meshPath = inputPath / "mesh";
        if (!fs::exists(inputPath) || !fs::is_directory(meshPath))
        {
            std::cout << "ERROR: MapLoader -> Directory does not exist or path invalid." << std::endl;
            exit(1);
        }

        InitWindow(300, 100, "Loading Map!");

        std::cout << "START: Constructing map into bin file. \n";

        std::cout << "START: Loading mesh data into resource manager. \n";
        for (const auto& entry : fs::directory_iterator(meshPath))
        {
            auto extension = entry.path().extension().string();
            std::ranges::transform(extension, extension.begin(), [](unsigned char c) { return std::tolower(c); });
            if (extension == ".obj" || extension == ".glb" || extension == ".gltf")
            {
                ResourceManager::GetInstance().ModelLoadFromFile(entry.path().string());
            }
        }
        std::cout << "FINISH: Loading mesh data into resource manager. \n";

        int slices = 0;

        std::cout << "START: Processing txt data into resource manager. \n";
        for (const auto& entry : fs::directory_iterator(inputPath))
        {
            if (entry.path().extension() == ".txt")
            {
                processTxtFile(registry, &itemFactory, meshPath, entry.path(), slices);
            }
        }
        std::cout << "FINISH: Processing txt data into resource manager. \n";

        ImageSafe heightMap(false), normalMap(false);

        navigationGridSystem->Init(slices, 1.0f);
        navigationGridSystem->InitGridHeightAndNormals();
        navigationGridSystem->GenerateHeightMap(heightMap);
        navigationGridSystem->GenerateNormalMap(normalMap);

        // ExportImage(heightMap.GetImage(), "resources/HeightMap.png");
        // ExportImage(normalMap.GetImage(), "resources/NormalMap.png");

        ResourceManager::GetInstance().ImageLoadFromFile("HEIGHT_MAP", heightMap.GetImage());
        ResourceManager::GetInstance().ImageLoadFromFile("NORMAL_MAP", normalMap.GetImage());

        lq::serializer::SaveMap(*registry, output);
        std::cout << "FINISH: Constructing map into bin file. \n";
    }

    /**
     * output: The path + filename of the resulting binary
     **/
    void ResourcePacker::PackAssets(entt::registry* registry, const std::string& output)
    {
        fs::path outputPath(output);
        if (!fs::is_directory(outputPath.parent_path()))
        {
            std::cout << "ResourcePacker: Directory does not exist, cannot save. Aborting... \n";
            return;
        }
        if (outputPath.extension() != ".bin")
        {
            std::cout << "ResourcePacker: File extension for output file must be 'bin'. Aborting... \n";
            return;
        }

        std::cout << "START: Loading assets into memory \n";
        {
            fs::path imagePath("resources/textures");
            if (!fs::is_directory(imagePath.parent_path()))
            {
                std::cout << "ResourcePacker: Image directory does not exist, cannot load. Aborting... \n";
                return;
            }
            std::cout << "START: Processing image data into resource manager. \n";
            for (const auto& entry : fs::recursive_directory_iterator(imagePath))
            {
                if (entry.path().extension() == ".png")
                {
                    ResourceManager::GetInstance().ImageLoadFromFile(entry.path());
                }
            }
            std::cout << "FINISH: Processing image data into resource manager. \n";
        }
        {
            fs::path iconsPath("resources/icons");
            if (!fs::is_directory(iconsPath.parent_path()))
            {
                std::cout << "ResourcePacker: Icon directory does not exist, cannot load. Aborting... \n";
                return;
            }
            std::cout << "START: Processing icon data into resource manager. \n";
            for (const auto& entry : fs::recursive_directory_iterator(iconsPath))
            {
                if (entry.path().extension() == ".png")
                {
                    ResourceManager::GetInstance().ImageLoadFromFile(entry.path());
                }
            }
            std::cout << "FINISH: Processing icon data into resource manager. \n";
        }
        {
            fs::path iconsPath("resources/fonts");
            if (!fs::is_directory(iconsPath.parent_path()))
            {
                std::cout << "ResourcePacker: Font directory does not exist, cannot load. Aborting... \n";
                return;
            }
            std::cout << "START: Processing font data into resource manager. \n";
            for (const auto& entry : fs::recursive_directory_iterator(iconsPath))
            {
                if (entry.path().extension() == ".ttf")
                {
                    ResourceManager::GetInstance().FontLoadFromFile(entry.path());
                }
            }
            std::cout << "FINISH: Processing font data into resource manager. \n";
        }
        {
            fs::path modelPath("resources/models");
            if (!fs::is_directory(modelPath.parent_path()))
            {
                std::cout << "ResourcePacker: Model directory does not exist, cannot load. Aborting... \n";
                return;
            }
            std::cout << "START: Processing model data into resource manager. \n";
            constexpr std::array<std::string, 3> validExtensions = {".glb", ".gltf", ".obj"};
            for (const auto& entry : fs::recursive_directory_iterator(modelPath))
            {
                if (std::find(validExtensions.begin(), validExtensions.end(), entry.path().extension()) ==
                    validExtensions.end())
                {
                    continue;
                }
                ResourceManager::GetInstance().ModelLoadFromFile(entry.path());
                if (entry.path().extension() == ".glb" || entry.path().extension() == ".gltf")
                {
                    ResourceManager::GetInstance().ModelAnimationLoadFromFile(entry.path());
                }
            }
            std::cout << "FINISH: Processing image data into resource manager. \n";
        }

        // Currently, serialization of music/sound is not supported
        // ResourceManager::GetInstance().MusicLoadFromFile("resources/audio/bgs/Cave.ogg");
        // ResourceManager::GetInstance().MusicLoadFromFile("resources/audio/music/bgm.ogg");
        // ResourceManager::GetInstance().SFXLoadFromFile("resources/audio/sfx/chest_open.ogg");
        // ResourceManager::GetInstance().SFXLoadFromFile("resources/audio/sfx/inv_open.ogg");
        // ResourceManager::GetInstance().SFXLoadFromFile("resources/audio/sfx/inv_close.ogg");
        // ResourceManager::GetInstance().SFXLoadFromFile("resources/audio/sfx/book_open.ogg");
        // ResourceManager::GetInstance().SFXLoadFromFile("resources/audio/sfx/equip_open.ogg");

        std::cout << "FINISH: Loading assets into memory \n";
        serializer::SaveClassBinary(output.c_str(), ResourceManager::GetInstance());
    }
}; // namespace sage