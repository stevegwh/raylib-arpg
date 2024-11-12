//
// Created by Steve Wheeler on 03/10/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
    class PartySystem;
    class ControllableActorSystem;
    struct Ability;
    struct ItemComponent;
    class Window;
    class GameUIEngine;
    class PlayerAbilitySystem;
    struct CombatableActor;
    struct DialogComponent;

    class GameUiFactory
    {

      public:
        static void CreateExampleWindow(GameUIEngine* engine);
        static Window* CreatePartyPortraitsColumn(GameUIEngine* engine);
        static Window* CreateAbilityRow(GameUIEngine* engine);
        static Window* CreateInventoryWindow(
            entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h);
        static Window* CreateCharacterWindow(
            entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h);
        static Window* CreateWorldTooltip(GameUIEngine* engine, const std::string& name, Vector2 pos);
        static Window* CreateCombatableTooltip(
            GameUIEngine* engine, const std::string& name, CombatableActor& combatInfo, Vector2 pos);
        static Window* CreateItemTooltip(GameUIEngine* engine, ItemComponent& item, Vector2 pos);
        static Window* CreateAbilityToolTip(GameUIEngine* engine, const Ability& ability, Vector2 pos);
        static Window* CreateDialogWindow(GameUIEngine* engine, entt::entity npc);
    };

} // namespace sage
