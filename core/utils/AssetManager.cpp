#include "AssetManager.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include "raylib-cereal.hpp"
#include "raylib.h"
#include <cassert>
#include <cereal/archives/json.hpp>
#include "entt/entt.hpp"
#include <fstream>
#include <magic_enum.hpp>
#include <type_traits>
#include <vector>

namespace sage
{

    void AssetManager::addAsset(const AssetID& asset, const std::string& path)
    {
        assert(!assetMap.contains(asset));
        assert(FileExists(path.c_str()));
        assetMap.emplace(asset, path);
    }

    const std::string& AssetManager::GetAssetPath(const AssetID& assetKey)
    {
        assert(assetMap.contains(assetKey));
        return assetMap.at(assetKey);
    }

    const std::string& AssetManager::TryGetAssetPath(const std::string& assetKey)
    {
        if (assetMap.contains(assetKey))
        {
            return assetMap.at(assetKey);
        }
        return assetKey;
    }

    void AssetManager::LoadPaths() const
    {
        if (!assetMap.empty())
        {
            std::cout << "AssetManager: Map already loaded. Aborting... \n";
            return;
        }
        std::cout << "START: Loading asset paths from JSON file \n";

        if (!FileExists(ASSET_JSON))
        {
            std::cout << "WARNING: No asset path file detected. \n";
            assert(0);
        }

        using namespace entt::literals;
        std::ifstream storage(ASSET_JSON);
        if (storage.is_open())
        {
            cereal::JSONInputArchive input{storage};
            input(GetInstance());
            storage.close();
        }
        std::cout << "FINISH: Loading asset paths from JSON file \n";
    }

}; // namespace sage