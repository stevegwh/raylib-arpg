#include "MapLoader.hpp"

int main(int argc, char* argv[])
{
    sage::MapLoader mapLoader;
    entt::registry registry{};
    // Should take path and other things as arguments/flags
    sage::MapLoader::ConstructMap(&registry, "resources/maps/level");
    return 0;
}