//
// Created by Steve Wheeler on 15/01/2025.
//

#include "LootSystem.hpp"

#include "components/InventoryComponent.hpp"
#include "components/Renderable.hpp"
#include "Cursor.hpp"
#include "GameUiFactory.hpp"
#include "ItemFactory.hpp"
#include "Systems.hpp"

#include "raylib.h"

namespace sage
{

    void LootSystem::onChestClick(entt::entity chest)
    {
        GameUiFactory::CreateLootWindow(registry, sys->uiEngine.get(), chest, GetMousePosition());
    }

    LootSystem::LootSystem(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
        sys->cursor->onChestClick.Subscribe([this](entt::entity _chest) { onChestClick(_chest); });
    }

} // namespace sage