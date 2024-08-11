#pragma once

#include <entt/entt.hpp>

namespace sage
{
    enum class PlayerStateEnum
    {
        Default,
        MovingToAttackEnemy,
        MovingToTalkToNPC,
        Combat
    };

    class PlayerState
    {
        PlayerStateEnum currentState = PlayerStateEnum::Default;

      public:
        // self, old state, new state
        entt::sigh<void(entt::entity, PlayerStateEnum, PlayerStateEnum)> onStateChanged;

        void ChangeState(entt::entity self, PlayerStateEnum newState)
        {
            onStateChanged.publish(self, currentState, newState);
            currentState = newState;
        }

        PlayerStateEnum GetCurrentState() const
        {
            return currentState;
        }
    };
} // namespace sage