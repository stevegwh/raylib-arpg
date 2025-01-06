//
// Created by Steve Wheeler on 06/01/2025.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{

    // Run at the end of the game loop
    class CleanupSystem
    {

        entt::registry* registry;

      public:
        void Execute() const;
        explicit CleanupSystem(entt::registry* _registry);
    };

} // namespace sage
