#pragma once

#include "AssetID.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/unordered_map.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include "magic_enum.hpp"
#include "raylib.h"

#include <cereal/archives/json.hpp>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace cereal
{
    template <class Archive>
    inline std::string save_minimal(Archive const&, const sage::AssetID& t)
    {
        return std::string(magic_enum::enum_name(t));
    }

    template <class Archive>
    inline void load_minimal(Archive const&, sage::AssetID& t, std::string const& value)
    {
        t = magic_enum::enum_cast<sage::AssetID>(value).value();
    }
} // namespace cereal

namespace sage
{

    class AssetManager
    {
        static constexpr std::string jsonPath = "resources/asset-paths.json";
        std::unordered_map<AssetID, std::string> assetMap;
        AssetManager() = default;
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