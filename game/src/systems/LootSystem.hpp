//
// Created by Steve Wheeler on 15/01/2025.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class Window;
}

namespace lq
{

    class Systems;

    static constexpr auto LOOT_DISTANCE = 30;

    class LootSystem
    {
        entt::registry* registry;
        Systems* sys;
        entt::entity chest;
        sage::Window* openLootWindow;
        bool windowRemovedExternally = false;

      public:
        void OnChestClick(entt::entity clickedChest);
        [[nodiscard]] bool InLootRange() const;
        void Update();
        LootSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace lq
