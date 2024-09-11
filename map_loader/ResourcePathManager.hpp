#pragma once

#include <string>
#include <unordered_map>

namespace sage
{
    enum class ResEnum
    {
        MDL_PLAYER_DEFAULT,
        MDL_NPC_ARISSA,
        MDL_ENEMY_GOBLIN,
        MDL_BUILDING_PORTAL,
        MDL_BUILDING_WIZARDTOWER1,
        IMG_CURSOR_REGULAR,
        IMG_CURSOR_TALK,
        IMG_CURSOR_MOVE,
        IMG_CURSOR_DENIED,
        IMG_CURSOR_ATTACK,
        IMG_APPLICATIONICON,
        TEX_IMGCIRCLE16,
        TEX_IMGCIRCLE8,
        TEX_SPARKFLAME,
        TEX_NOISE50,
        TEX_NOISE45
    };

    class ResourcePathManager()
    {
        std::unordered_map<ResEnum, std::string> paths;

        ResourcePathManager();
        ~ResourcePathManager() = default;

      public:
        static ResourcePathManager& GetInstance()
        {
            static ResourcePathManager instance;
            return instance;
        }

        void AddResource(ResEnum res, const std::string& path);
        const std::string& GetResource(ResEnum res);
    }

}; // namespace sage