//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include <array>
#include <entt/entt.hpp>

namespace sage
{
    struct InventoryComponent
    {
        static constexpr int MAX_ROW = 4;
        static constexpr int MAX_COL = 4;
        std::array<std::array<entt::entity, MAX_COL>, MAX_ROW> items;
        void AddItem(entt::entity entity, int slotNumber);
        void RemoveItem(int slotNumber);
        void UseItem(int slotNumber);
    };

} // namespace sage
