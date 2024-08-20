#pragma once

#include <entt/entt.hpp>
#include <Timer.hpp>

namespace sage
{
    class AbilityState
    {
      public:
        Timer& cooldownTimer;
        Timer& animationDelayTimer;

        virtual void Update(entt::entity self);
        virtual void Draw3D(entt::entity self);
        virtual void OnEnter(entt::entity self);
        virtual void OnExit(entt::entity self);
        virtual ~AbilityState();
        AbilityState(Timer& cooldownTimer, Timer& animationDelayTimer);
    };
} // namespace sage