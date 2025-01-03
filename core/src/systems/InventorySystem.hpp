//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    class Window;

    class InventorySystem
    {
        struct LastHit
        {
            Vector3 pos;
            bool reachable = false;
        };

        entt::registry* registry;
        GameData* gameData;
        LastHit lastWorldItemHovered;
        void onWorldItemClicked(entt::entity entity);
        void inventoryUpdated() const;
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        [[nodiscard]] bool CheckWorldItemRange();
        entt::sigh<void()> onInventoryUpdated;
        InventorySystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
