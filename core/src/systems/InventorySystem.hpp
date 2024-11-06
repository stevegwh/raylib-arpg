//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    class InventorySystem
    {
        entt::registry* registry;
        GameData* gameData;
        void inventoryUpdated() const;
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        void OnItemClicked(entt::entity entity) const;
        entt::sigh<void()> onInventoryUpdated;
        InventorySystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
