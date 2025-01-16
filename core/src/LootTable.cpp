//
// Created by Steve Wheeler on 30/11/2024.
//

#include "LootTable.hpp"

#include "components/InventoryComponent.hpp"
#include "ItemFactory.hpp"
#include "Systems.hpp"
#include "systems/RenderSystem.hpp"

namespace sage
{

    void LootTable::loadItems(const std::unordered_map<std::string, std::vector<std::string>>& lootTable) const
    {
        for (const auto& [k, v] : lootTable)
        {
            const auto entity =
                sys->renderSystem->FindRenderable("_CHEST_" + k); // Not a huge fan of adding the tag
            assert(entity != entt::null);
            auto& inventory = registry->get<InventoryComponent>(entity);
            for (const auto& item : v)
            {
                const auto itemId = sys->itemFactory->GetItem(item);
                if (!inventory.AddItem(itemId)) assert(0);
            }
        }
    }

    LootTable::LootTable(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
    }
} // namespace sage