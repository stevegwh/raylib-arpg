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

      public:
        void OnItemClicked(entt::entity entity) const;
        InventorySystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
