//
// Created by Steve Wheeler on 15/01/2025.
//

#include "LootSystem.hpp"

#include "components/InventoryComponent.hpp"
#include "components/Renderable.hpp"
#include "ControllableActorSystem.hpp"
#include "Cursor.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"
#include "ItemFactory.hpp"
#include "Systems.hpp"

#include "components/sgTransform.hpp"
#include "raylib.h"

namespace sage
{

    void LootSystem::onChestClick(entt::entity clickedChest)
    {
        if (openLootWindow)
        {
            openLootWindow->Remove();
            openLootWindow = nullptr;
        }
        if (clickedChest != chest)
        {
            openLootWindow =
                GameUiFactory::CreateLootWindow(registry, sys->uiEngine.get(), clickedChest, GetMousePosition());
            openLootWindow->onHide.Subscribe([this] { openLootWindow = nullptr; });
            chest = clickedChest;
            return;
        }
        chest = entt::null;
    }

    bool LootSystem::InLootRange() const
    {
        if (chest == entt::null) return false;
        // If the player walks too far from the lootable object, then remove the loot window.
        const auto& transform = registry->get<sgTransform>(chest).GetWorldPos();
        const auto& playerPos =
            registry->get<sgTransform>(sys->controllableActorSystem->GetSelectedActor()).GetWorldPos();
        return Vector3Distance(playerPos, transform) > LOOT_DISTANCE;
    }

    void LootSystem::Update()
    {
        if (!InLootRange())
        {
            openLootWindow->Remove();
            chest = entt::null;
        }
    }

    LootSystem::LootSystem(entt::registry* _registry, Systems* _sys)
        : registry(_registry), sys(_sys), openLootWindow(nullptr), chest(entt::null)
    {
        // TODO: Put below in PlayerStateSystem and make a state for chest interaction based on distance etc.
        sys->cursor->onChestClick.Subscribe([this](entt::entity _chest) { onChestClick(_chest); });
    }

} // namespace sage