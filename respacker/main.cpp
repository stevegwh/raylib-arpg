#include "engine/ResourceManager.hpp"
#include "engine/systems/CollisionSystem.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "engine/systems/TransformSystem.hpp"
#include "ResourcePacker.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    InitWindow(300, 100, "Packing Assets...");
    entt::registry registry{};
    sage::TransformSystem transformSystem(&registry);
    sage::CollisionSystem collisionSystem(&registry);
    sage::NavigationGridSystem navigationGridSystem(&registry, &collisionSystem);

    if (argc > 1)
    {
        const std::string command = argv[1];
        if (command == "--pack-assets")
        {
            sage::ResourcePacker::PackAssets(&registry, argc > 2 ? argv[2] : "resources/assets.bin");
        }
        else if (command == "--construct-map")
        {
            sage::ResourcePacker::ConstructMap(
                &registry,
                &navigationGridSystem,
                &transformSystem,
                argc > 2 ? argv[2] : "resources/maps/dungeon-map",
                argc > 3 ? argv[3] : "resources/dungeon-map.bin");
        }
        else if (command == "--export-editor-assets")
        {
            sage::ResourcePacker::ExportEditorAssetsFromMapBin(
                &registry,
                &transformSystem,
                argc > 2 ? argv[2] : "resources/dungeon-map.bin",
                argc > 3 ? argv[3] : "resources/editor-map-assets.bin");
        }
        else
        {
            std::cerr << "Unknown respacker command: " << command << std::endl;
            CloseWindow();
            return 1;
        }

        CloseWindow();
        return 0;
    }

    // clang-format off
    sage::ResourcePacker::PackAssets(&registry, "resources/assets.bin");
    sage::ResourcePacker::ConstructMap( &registry, &navigationGridSystem, &transformSystem, "resources/maps/dungeon-map", "resources/dungeon-map.bin");
    sage::ResourcePacker::ExportEditorAssetsFromMapBin(&registry, &transformSystem, "resources/dungeon-map.bin", "resources/editor-map-assets.bin");
    // clang-format on

    CloseWindow();

    return 0;
}
