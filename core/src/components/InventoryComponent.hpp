//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include <array>
#include <entt/entt.hpp>

namespace sage
{
    static constexpr unsigned int INVENTORY_MAX_ROWS = 4;
    static constexpr unsigned int INVENTORY_MAX_COLS = 4;
    class InventoryComponent
    {
        [[nodiscard]] bool getNextFreeSlot(unsigned int& row, unsigned int& col) const;
        std::array<std::array<entt::entity, INVENTORY_MAX_COLS>, INVENTORY_MAX_ROWS> items{}; // ItemComponent etc

      public:
        entt::sigh<void()> onItemAdded;
        entt::sigh<void()> onItemRemoved;
        entt::sigh<void()> onInventoryFull;
        [[nodiscard]] bool AddItem(entt::entity entity);
        bool AddItem(entt::entity entity, unsigned int row, unsigned int col);
        void RemoveItem(unsigned int row, unsigned int col);
        void SwapItems(unsigned int row1, unsigned int col1, unsigned int row2, unsigned int col2);
        entt::entity GetItem(unsigned int row, unsigned int col);
        InventoryComponent();
    };

} // namespace sage
