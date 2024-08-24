#pragma once

#include <Timer.hpp>

#include <entt/entt.hpp>

#include <vector>

namespace sage
{
    class TimerSystem
    {
        entt::registry* registry;

      public:
        void Update();
        TimerSystem(entt::registry* _registry);
    };
} // namespace sage