//
// Created by Steve Wheeler on 08/10/2024.
//

#include "InventoryComponent.hpp"

namespace sage
{

    void InventoryComponent::AddItem(entt::entity entity, unsigned int row, unsigned int col)
    {
        if (items[row][col] != entt::null) return;
        items[row][col] = entity;
    }

    void InventoryComponent::RemoveItem(unsigned int row, unsigned int col)
    {
    }

    entt::entity InventoryComponent::GetItem(unsigned int row, unsigned int col)
    {
        return items[row][col];
    }

    InventoryComponent::InventoryComponent()
    {
        // I believe this is necessary;
        for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
        {
            for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
            {
                items[row][col] = entt::null;
            }
        }
    }
} // namespace sage