//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "Event.hpp"

#include <array>
#include <entt/entt.hpp>
#include <memory>

namespace sage
{
    static constexpr unsigned int INVENTORY_MAX_ROWS = 7;
    static constexpr unsigned int INVENTORY_MAX_COLS = 5;
    class InventoryComponent
    {
        [[nodiscard]] bool getNextFreeSlot(unsigned int& row, unsigned int& col) const;
        std::array<std::array<entt::entity, INVENTORY_MAX_COLS>, INVENTORY_MAX_ROWS> items{}; // ItemComponent etc

      public:
        std::shared_ptr<Connection<>> onItemAddedCnx;
        std::shared_ptr<Connection<>> onItemRemovedCnx;
        std::shared_ptr<Connection<>> onInventoryFullCnx;

        std::unique_ptr<Event<>> onItemAdded;
        std::unique_ptr<Event<>> onItemRemoved;
        std::unique_ptr<Event<>> onInventoryFull;
        [[nodiscard]] bool AddItem(entt::entity entity);
        bool AddItem(entt::entity entity, unsigned int row, unsigned int col);
        void RemoveItem(unsigned int row, unsigned int col);
        void RemoveItem(entt::entity item);
        void SwapItems(unsigned int row1, unsigned int col1, unsigned int row2, unsigned int col2);
        [[nodiscard]] entt::entity GetItem(unsigned int row, unsigned int col) const;
        InventoryComponent();
    };

} // namespace sage
