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
#include <entt/entt.hpp>
#include <fstream>
#include <magic_enum.hpp>
#include <type_traits>
#include <vector>

namespace sage
{

    void AssetManager::addAsset(AssetID asset, const std::string& path)
    {
        assert(!assetMap.contains(asset));
        // Could check if path has been referenced by other object. Easy to do with a two-way map
        assert(FileExists(path.c_str()));
        assetMap.emplace(asset, path);
    }

    const std::string& AssetManager::GetAssetPath(AssetID asset)
    {
        return assetMap.at(asset);
    }

    void AssetManager::GenerateBlankJson()
    {
        for (int i = 0; i < magic_enum::enum_underlying(AssetID::COUNT); ++i)
        {
            auto id = magic_enum::enum_cast<AssetID>(i).value();
            assetMap.emplace(id, "");
        }
        SavePaths();
    }

    void AssetManager::SavePaths()
    {
        std::cout << "START: Saving asset paths to JSON file \n";
        using namespace entt::literals;
        if (FileExists(ASSET_JSON))
        {
            auto file = LoadFileText(ASSET_JSON);
            SaveFileText(std::string(std::string(ASSET_JSON) + ".bak").c_str(), file);
            UnloadFileText(file);
        }

        std::ofstream storage(ASSET_JSON);
        if (!storage.is_open())
        {
            return;
        }

        {
            cereal::JSONOutputArchive output{storage};
            output.setNextName("AssetManager");
            output(GetInstance());
        }
        storage.close();
        std::cout << "FINISH: Saving asset paths to JSON file \n";
    }

    void AssetManager::LoadPaths()
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