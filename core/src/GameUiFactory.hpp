//
// Created by Steve Wheeler on 03/10/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
    class ControllableActorSystem;
    struct Ability;
    struct ItemComponent;
    struct Window;
    class GameUIEngine;
    class PlayerAbilitySystem;

    class GameUiFactory
    {

      public:
        static void CreateExampleWindow(GameUIEngine* engine);
        static void CreateAbilityRow(GameUIEngine* engine, PlayerAbilitySystem* playerAbilitySystem);
        [[nodiscard]] static Window* CreateItemTooltip(GameUIEngine* engine, ItemComponent& item, Vector2 pos);
        [[nodiscard]] static Window* CreateAbilityToolTip(GameUIEngine* engine, const Ability& ability, Vector2 pos);
        static Window* CreateInventoryWindow(
            entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h);
    };

} // namespace sage
