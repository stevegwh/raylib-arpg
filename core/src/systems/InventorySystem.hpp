//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "Event.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class Systems;
    class Window;

    class InventorySystem
    {
        struct LastHit
        {
            Vector3 pos;
            bool reachable = false;
        };

        entt::registry* registry;
        Systems* sys;
        LastHit lastWorldItemHovered;
        void onWorldItemClicked(entt::entity entity);
        void inventoryUpdated() const;
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        [[nodiscard]] bool CheckWorldItemRange(bool hover = false);
        Event<> onInventoryUpdated;
        InventorySystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
