//
// Created by Steve Wheeler on 08/10/2024.
//

#include "InventoryComponent.hpp"

namespace sage
{

    bool InventoryComponent::getNextFreeSlot(unsigned int& row, unsigned int& col) const
    {
        row = 0;
        col = 0;
        for (row = 0; row < items.size(); ++row)
        {
            for (col = 0; col < items[row].size(); ++col)
            {
                if (items[row][col] == entt::null)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool InventoryComponent::AddItem(entt::entity entity)
    {
        unsigned int row = 0;
        unsigned int col = 0;
        if (!getNextFreeSlot(row, col)) return false;
        return AddItem(entity, row, col);
    }

    bool InventoryComponent::AddItem(entt::entity entity, unsigned int row, unsigned int col)
    {
        if (items[row][col] != entt::null) return false;
        items[row][col] = entity;
        onItemAdded.publish();
        return true;
    }

    void InventoryComponent::RemoveItem(unsigned int row, unsigned int col)
    {
        items[row][col] = entt::null;
        onItemRemoved.publish();
    }

    void InventoryComponent::RemoveItem(entt::entity item)
    {
        for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
        {
            for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
            {
                if (auto _item = items[row][col]; item == _item)
                {
                    RemoveItem(row, col);
                }
            }
        }
    }

    entt::entity InventoryComponent::GetItem(unsigned int row, unsigned int col) const
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