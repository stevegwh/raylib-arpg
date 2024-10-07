//
// Created by Steve Wheeler on 03/10/2024.
//

#pragma once

#include "raylib.h"

namespace sage
{
    class GameUIEngine;
    class PlayerAbilitySystem;

    class GameUiFactory
    {

      public:
        static void CreateExampleWindow(GameUIEngine* engine);
        static void CreateAbilityRow(GameUIEngine* engine, PlayerAbilitySystem* playerAbilitySystem);
        static void CreateInventoryWindow(GameUIEngine* engine, Vector2 pos, float w, float h);
    };

} // namespace sage
