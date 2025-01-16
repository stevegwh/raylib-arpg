//
// Created by Steve Wheeler on 15/01/2025.
//

#include "LootSystem.hpp"

#include "AudioManager.hpp"
#include "Camera.hpp"
#include "components/InventoryComponent.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "ControllableActorSystem.hpp"
#include "Cursor.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"
#include "Settings.hpp"
#include "Systems.hpp"

#include "raylib.h"

namespace sage
{

    void LootSystem::OnChestClick(entt::entity clickedChest)
    {
        auto createLootWindow = [&]() {
            auto& transform = registry->get<sgTransform>(clickedChest);
            auto screenPos = GetWorldToScreenEx(
                transform.GetWorldPos(),
                *sys->camera->getRaylibCam(),
                Settings::TARGET_SCREEN_WIDTH,
                Settings::TARGET_SCREEN_HEIGHT);
            openLootWindow =
                GameUiFactory::CreateLootWindow(registry, sys->uiEngine.get(), clickedChest, screenPos);
            openLootWindow->onHide.Subscribe([this] { openLootWindow = nullptr; });
            sys->audioManager->PlaySFX("resources/audio/sfx/chest_open.ogg");
        };

        // if the clicked chest differs from the previous, open a new window
        if (chest == clickedChest)
        {
            if (openLootWindow)
            {
                openLootWindow->Remove();
            }
            else
            {
                createLootWindow();
            }
        }
        else
        {
            if (openLootWindow)
            {
                openLootWindow->Remove();
            }
            createLootWindow();
            chest = clickedChest;
        }
    }

    bool LootSystem::InLootRange() const
    {
        if (chest == entt::null) return false;
        // If the player walks too far from the lootable object, then remove the loot window.
        const auto& transform = registry->get<sgTransform>(chest).GetWorldPos();
        const auto& playerPos =
            registry->get<sgTransform>(sys->controllableActorSystem->GetSelectedActor()).GetWorldPos();
        return Vector3Distance(playerPos, transform) < LOOT_DISTANCE;
    }

    void LootSystem::Update()
    {
        if (chest == entt::null || !openLootWindow) return;
        if (!InLootRange())
        {
            openLootWindow->Remove();
            openLootWindow = nullptr;
            chest = entt::null;
        }
    }

    LootSystem::LootSystem(entt::registry* _registry, Systems* _sys)
        : registry(_registry), sys(_sys), chest(entt::null), openLootWindow(nullptr)
    {
    }

} // namespace sage