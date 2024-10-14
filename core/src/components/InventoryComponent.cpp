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
        items[row][col] = entt::null;
    }

    entt::entity InventoryComponent::GetItem(unsigned int row, unsigned int col)
    {
        return items[row][col];
    }

    void InventoryComponent::SwapItems(unsigned int row1, unsigned int col1, unsigned int row2, unsigned int col2)
    {
        auto item1 = items[row1][col1];
        auto item2 = items[row2][col2];
        items[row1][col1] = item2;
        items[row2][col2] = item1;
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