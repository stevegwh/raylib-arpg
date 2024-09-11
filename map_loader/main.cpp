#include "AssetManager.hpp"
#include "MapLoader.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

int main(int argc, char* argv[])
{
    sage::MapLoader mapLoader;
    entt::registry registry{};
    sage::CollisionSystem collisionSystem(&registry);
    sage::NavigationGridSystem navigationGridSystem(&registry, &collisionSystem);
    // Should take path and other things as arguments/flags
    sage::MapLoader::ConstructMap(&registry, &navigationGridSystem, "resources/maps/level");

    return 0;
}