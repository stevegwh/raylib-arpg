//
// Created by Steve Wheeler on 11/09/2024.
//

#pragma once

namespace sage
{
    enum class AssetID
    {
        MDL_PLAYER_DEFAULT,
        MDL_NPC_ARISSA,
        MDL_ENEMY_GOBLIN,
        MDL_BUILDING_PORTAL,
        MDL_BUILDING_WIZARDTOWER1,
        MDL_VFX_SPHERE,
        MDL_VFX_FLATTORUS,
        IMG_CURSOR_REGULAR,
        IMG_CURSOR_TALK,
        IMG_CURSOR_MOVE,
        IMG_CURSOR_DENIED,
        IMG_CURSOR_ATTACK,
        IMG_APPLICATIONICON,
        IMG_IMGCIRCLE16,
        IMG_IMGCIRCLE8,
        IMG_SPARKFLAME,
        IMG_NOISE59,
        IMG_NOISE50,
        IMG_NOISE45,
        IMG_NOISE53,
        IMG_RAINOFFIRE_CURSOR,
        // Generated and set internally
        GEN_IMG_HEIGHTMAP,
        GEN_IMG_NORMALMAP,
        COUNT // Should always be last (Used to generate blank JSON file)
    };

} // namespace sage