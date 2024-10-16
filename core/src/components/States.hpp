#pragma once

#include <entt/entt.hpp>

namespace sage
{
    template <typename StateName, typename StateEnum>
    class BaseState
    {
        StateEnum currentState;

      public:
        // self, new state
        entt::sigh<void(entt::entity, StateEnum)> onStateChanged;

        void ChangeState(entt::entity self, StateEnum newState)
        {
            onStateChanged.publish(self, newState);
            currentState = newState;
        }

        void SetState(StateEnum newState)
        {
            currentState = newState;
        }

        [[nodiscard]] StateEnum GetCurrentState() const
        {
            return currentState;
        }

        BaseState(const BaseState&) = delete;
        BaseState& operator=(const BaseState&) = delete;

        explicit BaseState(StateEnum initialState) : currentState(initialState)
        {
        }
    };

    enum class PlayerStateEnum
    {
        Default,
        MovingToAttackEnemy,
        MovingToTalkToNPC,
        Combat
    };

    class PlayerState : public BaseState<PlayerState, PlayerStateEnum>
    {
      public:
        PlayerState() : BaseState(PlayerStateEnum::Default)
        {
        }
    };

    enum class GameStateEnum
    {
        Default,
        Wave
    };

    class GameState : public BaseState<GameState, GameStateEnum>
    {
      public:
        GameState() : BaseState(GameStateEnum::Default)
        {
        }
    };

    enum class WavemobStateEnum
    {
        Default,
        TargetOutOfRange,
        Combat,
        Dying
    };

    class WavemobState : public BaseState<WavemobState, WavemobStateEnum>
    {
      public:
        WavemobState() : BaseState(WavemobStateEnum::Default)
        {
        }
    };

    enum class TowerStateEnum
    {
        Default,
        Combat
    };

    class TowerState : public BaseState<TowerState, TowerStateEnum>
    {
      public:
        TowerState() : BaseState(TowerStateEnum::Default)
        {
        }
    };

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class AbilityState : public BaseState<AbilityState, AbilityStateEnum>
    {
      public:
        AbilityState() : BaseState(AbilityStateEnum::IDLE)
        {
        }
    };

} // namespace sage