//
// Created by Steve Wheeler on 04/09/2024.
//

#pragma once

#include "AssetID.hpp"
#include "systems/NavigationGridSystem.hpp"

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

// Takes a gltf or obj file, instantiates it into game components and serializes it as a "bin" file

namespace sage
{
    class NavigationGridSystem;

    class MapLoader
    {
        std::unordered_map<std::string, AssetID> pathIdMap;

      public:
        static void ConstructMap(
            entt::registry* registry, NavigationGridSystem* navigationGridSystem, const char* path);
    };

} // namespace sage
