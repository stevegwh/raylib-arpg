#include "ResourcePathManager.hpp"
#include "raylib.h"

#include <cassert>

namespace sage
{

    void ResourcePathManager::addResource(ResEnum res, const std::string& path)
    {
        assert(!resources.contains(res));
        // Could check if path has been referenced by other object. Easy to do with a two-way map
        assert(FileExists(path.c_str()));
        resources.emplace(res, path);
    }

    const std::string& ResourcePathManager::GetResource(ResEnum res)
    {
        return resources.at(res);
    }

    void ResourcePathManager::GenerateBlankJson()
    {
        for (int i = 0; i < magic_enum::enum_underlying(ResEnum::COUNT); ++i)
        {
            auto e = magic_enum::enum_cast<ResEnum>(i).value();
            resources.emplace(e, "");
        }
        SavePaths();
    }

    void ResourcePathManager::SavePaths()
    {
        std::cout << "START: Saving resource paths to JSON file \n";
        using namespace entt::literals;
        std::string path = "resources/paths.json";
        if (FileExists(path.c_str()))
        {
            auto file = LoadFileText(path.c_str());
            SaveFileText(std::string(path + ".bak").c_str(), file);
            UnloadFileText(file);
        }

        std::ofstream storage(path);
        if (!storage.is_open())
        {
            return;
        }

        {
            cereal::JSONOutputArchive output{storage};
            output.setNextName("ResourcePathManager");
            output(GetInstance());
        }
        storage.close();
        std::cout << "FINISH: Saving resource paths to JSON file \n";
    }

    void ResourcePathManager::LoadPaths()
    {
        std::cout << "START: Loading resource paths from JSON file \n";
        std::string path = "resources/paths.json";

        if (!FileExists(path.c_str()))
        {
            std::cout << "WARNING: No resource path file detected. \n";
            assert(0);
        }

        using namespace entt::literals;
        std::ifstream storage(path);
        if (storage.is_open())
        {
            cereal::JSONInputArchive input{storage};
            input(GetInstance());
            storage.close();
        }
        std::cout << "FINISH: Loading resource paths from JSON file \n";
    }

}; // namespace sage