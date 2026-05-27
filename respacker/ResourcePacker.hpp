//
// Created by Steve Wheeler on 04/09/2024.
//

#pragma once

#include "entt/entt.hpp"
#include <string>

// Takes a gltf or obj file, instantiates it into game components and serializes it as a "bin" file

namespace sage
{
    class NavigationGridSystem;
    class TransformSystem;

    class ResourcePacker
    {
      public:
        static void ConstructMap(
            entt::registry* registry,
            NavigationGridSystem* navigationGridSystem,
            TransformSystem* transformSystem,
            const char* input,
            const char* output);

        static void PackAssets(entt::registry* registry, const std::string& output);

        static void ExportEditorAssetsFromMapBin(
            entt::registry* registry,
            TransformSystem* transformSystem,
            const char* inputMapBin,
            const char* outputAssetBin);
    };

} // namespace sage
