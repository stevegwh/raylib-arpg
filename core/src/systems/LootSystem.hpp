//
// Created by Steve Wheeler on 15/01/2025.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{

    class Systems;

    class LootSystem
    {
        entt::registry* registry;
        Systems* sys;

        void onChestClick(entt::entity chest);

      public:
        LootSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
