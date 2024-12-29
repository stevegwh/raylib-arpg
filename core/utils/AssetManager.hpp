#pragma once

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/unordered_map.hpp"
#include "common_types.hpp"
#include <string>
#include <unordered_map>

namespace sage
{

    // Maintains a list of asset keys to their resources path (or the path at the time of serialization).
    // Allows us to use a key throughout the code base and only change the path once in the JSON file if required.
    class AssetManager
    {
        static constexpr const char* ASSET_JSON = "resources/asset-path-aliases.json";
        std::unordered_map<AssetID, std::string> assetMap;
        AssetManager() = default;
        ~AssetManager() = default;
        void addAsset(const AssetID& asset, const std::string& path);

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

        const std::string& GetAssetPath(const AssetID& assetKey);
        const std::string& TryGetAssetPath(const std::string& assetKey);
        void LoadPaths() const;

        friend class ResourcePacker;
    };

}; // namespace sage