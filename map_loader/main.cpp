#include "MapLoader.hpp"
#include "ResourcePathManager.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#define RESOURCE_PATHS_JSON

int main(int argc, char* argv[])
{
    sage::ResourcePathManager::GetInstance().GenerateBlankJson();
    return 0;
    sage::MapLoader mapLoader;
    entt::registry registry{};
    sage::CollisionSystem collisionSystem(&registry);
    sage::NavigationGridSystem navigationGridSystem(&registry, &collisionSystem);
    // Should take path and other things as arguments/flags
    sage::MapLoader::ConstructMap(&registry, &navigationGridSystem, "resources/maps/level");

    return 0;
}