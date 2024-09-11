#include "ResourcePathManager.hpp"
#include "raylib.h"

#include <cassert>

namespace sage
{

    void ResourcePathManager::AddResource(ResEnum res, const std::string& path)
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

    ResourcePathManager::ResourcePathManager()
    {
        // Read from a JSON file so you don't have to recompile the entire project just to add a resource.
        AddResource(ResEnum::MDL_PLAYER_DEFAULT, "resources/MDLs/gltf/hero2.glb");
        AddResource(ResEnum::MDL_NPC_ARISSA, "resources/MDLs/gltf/arissa.glb");
        AddResource(ResEnum::MDL_ENEMY_GOBLIN, "resources/MDLs/gltf/goblin.glb");

        AddResource(ResEnum::MDL_BUILDING_PORTAL, "resources/MDLs/obj/portal.obj");
        AddResource(ResEnum::MDL_BUILDING_WIZARDTOWER1, "resources/MDLs/obj/Wizard Tower 1.obj");

        AddResource(ResEnum::IMG_CURSOR_REGULAR, "resources/textures/cursor/32/regular.png");
        AddResource(ResEnum::IMG_CURSOR_TALK, "resources/textures/cursor/32/talk.png");
        AddResource(ResEnum::IMG_CURSOR_MOVE, "resources/textures/cursor/32/move.png");
        AddResource(ResEnum::IMG_CURSOR_DENIED, "resources/textures/cursor/32/denied.png");
        AddResource(ResEnum::IMG_CURSOR_ATTACK, "resources/textures/cursor/32/attack.png");
        AddResource(ResEnum::IMG_APPLICATIONICON, "resources/icon.png");
        AddResource(ResEnum::TEX_IMGCIRCLE16, "resources/imgCircle16.png");
        AddResource(ResEnum::TEX_IMGCIRCLE8, "resources/imgCircle8.png");
        AddResource(ResEnum::TEX_SPARKFLAME, "resources/textures/spark_flame.png");
        AddResource(ResEnum::TEX_NOISE50, "resources/textures/luos/Noise_Gradients/T_Random_50.png");
        AddResource(ResEnum::TEX_NOISE45, "resources/textures/luos/Noise_Gradients/T_Random_45.png");
    }

}; // namespace sage