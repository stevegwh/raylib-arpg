#pragma once

#include <entt/entt.hpp>

namespace sage
{
    enum class GameStateEnum
    {
        Default,
        Wave
    };

    class GameState
    {
        GameStateEnum currentState = GameStateEnum::Default;

      public:
        // self, old state, new state
        entt::sigh<void(entt::entity, GameStateEnum, GameStateEnum)> onStateChanged;

        void ChangeState(entt::entity self, GameStateEnum newState)
        {
            onStateChanged.publish(self, currentState, newState);
            currentState = newState;
        }

        GameStateEnum GetCurrentState() const
        {
            return currentState;
        }
    };
} // namespace sage