#include "engine/ResourceManager.hpp"
#include "engine/systems/CollisionSystem.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "ResourcePacker.hpp"

int main(int argc, char* argv[])
{
    InitWindow(300, 100, "Packing Assets...");
    entt::registry registry{};
    sage::CollisionSystem collisionSystem(&registry);
    sage::NavigationGridSystem navigationGridSystem(&registry, &collisionSystem);

    // clang-format off
    sage::ResourcePacker::PackAssets(&registry, "resources/assets.bin");
    sage::ResourcePacker::ConstructMap( &registry, &navigationGridSystem, "resources/maps/dungeon-map", "resources/dungeon-map.bin");
    // clang-format on

    CloseWindow();

    return 0;
}