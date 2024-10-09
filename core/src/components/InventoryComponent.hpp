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
    struct InventoryComponent
    {
        std::array<std::array<entt::entity, INVENTORY_MAX_COLS>, INVENTORY_MAX_ROWS> items; // ItemComponent etc
        void AddItem(entt::entity entity, int slotNumber);
        void RemoveItem(int slotNumber);
        void UseItem(int slotNumber);
    };

} // namespace sage
