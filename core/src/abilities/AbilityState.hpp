#pragma once

#include <entt/entt.hpp>
#include <Timer.hpp>

namespace sage
{
    class AbilityState
    {
      public:
        entt::entity self;
        Timer& cooldownTimer;
        Timer& animationDelayTimer;

        virtual void Update();
        virtual void Draw3D();
        virtual void OnEnter();
        virtual void OnExit();
        virtual ~AbilityState();
        AbilityState(entt::entity self, Timer& cooldownTimer, Timer& animationDelayTimer);
    };
} // namespace sage