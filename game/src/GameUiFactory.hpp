//
// Created by Steve Wheeler on 03/10/2024.
//

#pragma once

#include "engine/GameUiEngine.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

namespace lq
{
    namespace dialog
    {
        class Conversation;
    }
    class PartySystem;
    class ControllableActorSystem;
    struct Ability;
    struct ItemComponent;
    class LeverUIEngine;
    class PlayerAbilitySystem;
    struct CombatableActor;
    struct DialogComponent;

    class GameUiFactory
    {

      public:
        static sage::Window* CreatePartyPortraitsColumn(LeverUIEngine* engine);
        static sage::Window* CreateAbilityRow(LeverUIEngine* engine);
        static sage::Window* CreateLootWindow(
            entt::registry* registry, LeverUIEngine* engine, entt::entity owner, Vector2 pos);
        static sage::Window* CreateInventoryWindow(
            entt::registry* registry, LeverUIEngine* engine, Vector2 pos, float w, float h);
        static sage::Window* CreateJournalWindow(
            entt::registry* registry, LeverUIEngine* engine, Vector2 pos, float w, float h);
        static sage::Window* CreateCharacterWindow(
            entt::registry* registry, LeverUIEngine* engine, Vector2 pos, float w, float h);
        static sage::TooltipWindow* CreateWorldTooltip(
            LeverUIEngine* engine, const std::string& name, Vector2 pos);
        static sage::TooltipWindow* CreateItemTooltip(
            LeverUIEngine* engine, ItemComponent& item, sage::Window* parentWindow, Vector2 pos);
        static sage::TooltipWindow* CreateAbilityToolTip(
            LeverUIEngine* engine, const Ability& ability, Vector2 pos);
        static sage::Window* CreateDialogWindow(LeverUIEngine* engine, entt::entity npc);
        static sage::Window* CreateGameWindowButtons(
            LeverUIEngine* engine,
            sage::Window* inventoryWindow,
            sage::Window* equipmentWindow,
            sage::Window* journalWindow);
    };

} // namespace lq
