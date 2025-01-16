//
// Created by Steve Wheeler on 03/10/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    namespace dialog
    {
        class Conversation;
    }
    class PartySystem;
    class ControllableActorSystem;
    struct Ability;
    struct ItemComponent;
    class Window;
    class TooltipWindow;
    class GameUIEngine;
    class PlayerAbilitySystem;
    struct CombatableActor;
    struct DialogComponent;

    class GameUiFactory
    {

      public:
        static Window* CreatePartyPortraitsColumn(GameUIEngine* engine);
        static Window* CreateAbilityRow(GameUIEngine* engine);
        static Window* CreateLootWindow(
            entt::registry* registry, GameUIEngine* engine, entt::entity owner, Vector2 pos);
        static Window* CreateInventoryWindow(
            entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h);
        static Window* CreateJournalWindow(
            entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h);
        static Window* CreateCharacterWindow(
            entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h);
        static TooltipWindow* CreateWorldTooltip(GameUIEngine* engine, const std::string& name, Vector2 pos);
        static TooltipWindow* CreateItemTooltip(
            GameUIEngine* engine, ItemComponent& item, Window* parentWindow, Vector2 pos);
        static TooltipWindow* CreateAbilityToolTip(GameUIEngine* engine, const Ability& ability, Vector2 pos);
        static Window* CreateDialogWindow(GameUIEngine* engine, entt::entity npc);
        static Window* CreateGameWindowButtons(
            GameUIEngine* engine, Window* inventoryWindow, Window* equipmentWindow, Window* journalWindow);
    };

} // namespace sage
