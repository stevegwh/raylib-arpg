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

    struct PlayerState
    {
        PlayerStateEnum currentState = PlayerStateEnum::Default;

        // self, old state, new state
        entt::sigh<void(entt::entity, PlayerStateEnum, PlayerStateEnum)> onStateChanged;

        void ChangeState(entt::entity self, PlayerStateEnum newState)
        {
            onStateChanged.publish(self, currentState, newState);
            currentState = newState;
        }
    };
} // namespace sage