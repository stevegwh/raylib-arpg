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
        MDL_WPN_DAGGER01,
        MDL_WPN_SWORD01,
        IMG_ICON_WEAPON_DAGGER01,
        IMG_ICON_WEAPON_SWORD01,
        IMG_UI_CLOSE,
        IMG_CURSOR_REGULAR,
        IMG_CURSOR_TALK,
        IMG_CURSOR_MOVE,
        IMG_CURSOR_DENIED,
        IMG_CURSOR_ATTACK,
        IMG_CURSOR_PICKUP,
        IMG_APPLICATIONICON,
        IMG_IMGCIRCLE16,
        IMG_IMGCIRCLE8,
        IMG_SPARKFLAME,
        IMG_NOISE59,
        IMG_NOISE50,
        IMG_NOISE45,
        IMG_NOISE53,
        IMG_RAINOFFIRE_CURSOR,
        IMG_PORTRAIT_01,
        IMG_PORTRAIT_02,
        IMG_PORTRAIT_03,
        IMG_PORTRAIT_04,
        IMG_PORTRAIT_05,
        // Generated and set internally
        GEN_IMG_HEIGHTMAP,
        GEN_IMG_NORMALMAP,
        COUNT // Should always be last (Used to generate blank JSON file)
    };

} // namespace sage