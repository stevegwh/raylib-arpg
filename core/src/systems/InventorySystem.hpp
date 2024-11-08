//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    class Window;

    class InventorySystem
    {
        entt::registry* registry;
        GameData* gameData;
        Window* tooltipWindow;
        void onWorldItemStopHover() const;
        void onWorldItemHovered(entt::entity entity);
        void onWorldItemClicked(entt::entity entity) const;
        [[nodiscard]] bool checkWorldItemRange() const;
        void inventoryUpdated() const;
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        entt::sigh<void()> onInventoryUpdated;
        InventorySystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
