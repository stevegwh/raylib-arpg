#include "AssetManager.hpp"
#include "ResourcePacker.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

int main(int argc, char* argv[])
{
    InitWindow(300, 100, "Packing Assets...");

    sage::AssetManager::GetInstance().LoadPaths();
    sage::ResourcePacker mapLoader;
    entt::registry registry{};
    sage::CollisionSystem collisionSystem(&registry);
    sage::NavigationGridSystem navigationGridSystem(&registry, &collisionSystem);

    // clang-format off
   // sage::ResourcePacker::PackAssets(&registry, "resources/assets.bin");
     sage::ResourcePacker::ConstructMap( &registry, &navigationGridSystem, "resources/maps/dungeon-map", "resources/dungeon-map.bin");
    // clang-format on

    CloseWindow();

    return 0;
}