#include "TimerSystem.hpp"

#include "raylib.h"

namespace sage
{
    void TimerSystem::Update()
    {
        auto view = registry->view<Timer>();
        for (auto& entity : view)
        {
            auto& timer = registry->get<Timer>(entity);
            timer.Update(GetFrameTime());
        }
    }

    TimerSystem::TimerSystem(entt::registry* _registry) : registry(_registry)
    {
    }
} // namespace sage