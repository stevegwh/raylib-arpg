#pragma once

#include "ResEnum.hpp"

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
    inline std::string save_minimal(Archive const&, const sage::ResEnum& t)
    {
        return std::string(magic_enum::enum_name(t));
    }

    template <class Archive>
    inline void load_minimal(Archive const&, sage::ResEnum& t, std::string const& value)
    {
        t = magic_enum::enum_cast<sage::ResEnum>(value).value();
    }
} // namespace cereal

namespace sage
{

    class ResourcePathManager
    {
        std::unordered_map<ResEnum, std::string> resources;

        ResourcePathManager() = default;
        ~ResourcePathManager() = default;
        void addResource(ResEnum res, const std::string& path);

      public:
        static ResourcePathManager& GetInstance()
        {
            static ResourcePathManager instance;
            return instance;
        }

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(resources));
        }

        const std::string& GetResource(ResEnum res);
        void GenerateBlankJson();
        static void SavePaths();
        static void LoadPaths();
    };

}; // namespace sage