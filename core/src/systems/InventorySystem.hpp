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
        void onWorldItemClicked(entt::entity entity) const;
        void inventoryUpdated() const;
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        [[nodiscard]] bool CheckWorldItemRange() const;
        entt::sigh<void()> onInventoryUpdated;
        InventorySystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
