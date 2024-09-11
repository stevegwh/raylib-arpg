#pragma once

#include "AssetID.hpp"

#include "raylib-cereal.hpp"

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/unordered_map.hpp"
#include "magic_enum.hpp"
#include <string>
#include <unordered_map>

namespace sage
{

    class AssetManager
    {
        static constexpr const char* jsonPath = "resources/asset-paths.json";
        std::unordered_map<AssetID, std::string> assetMap;
        AssetManager();
        ~AssetManager() = default;
        void addAsset(AssetID asset, const std::string& path);

      public:
        static AssetManager& GetInstance()
        {
            static AssetManager instance;
            return instance;
        }

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(assetMap));
        }

        const std::string& GetAssetPath(AssetID asset);
        void GenerateBlankJson();
        static void SavePaths();
        static void LoadPaths();
    };

}; // namespace sage