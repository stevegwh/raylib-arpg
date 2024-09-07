//
// Created by Steve Wheeler on 04/09/2024.
//

#pragma once

#include <entt/entt.hpp>

// Takes a gltf or obj file, instantiates it into game components and serializes it as a "bin" file

namespace sage
{

    class MapLoader
    {
      public:
        static void ConstructMap(entt::registry* registry, const char* path);
    };

} // namespace sage
