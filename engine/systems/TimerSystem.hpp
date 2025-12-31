#pragma once

#include "entt/entt.hpp"

#include <vector>

namespace sage
{
    class TimerSystem
    {
        entt::registry* registry;

      public:
        void Update() const;
        TimerSystem(entt::registry* _registry);
    };
} // namespace sage