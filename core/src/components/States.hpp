#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <Event.hpp>
#include <vector>

namespace sage
{
    template <typename StateName, typename StateEnum>
    class BaseState
    {
        StateEnum currentState;
        std::vector<std::unique_ptr<Connection>> currentStateConnections;

      public:
        // All connections added here will be removed when ChangeState is called (via StateMachine).
        void ManageSubscription(std::unique_ptr<Connection> newConnection)
        {
            currentStateConnections.push_back(std::move(newConnection));
        }

        void RemoveAllConnections()
        {
            for (auto& connection : currentStateConnections)
            {
                connection->UnSubscribe();
            }
            currentStateConnections.clear();
        }

        void SetState(StateEnum newState)
        {
            currentState = newState;
        }

        [[nodiscard]] StateEnum GetCurrentState() const
        {
            return currentState;
        }

        ~BaseState()
        {
            RemoveAllConnections();
        }

        // Allow moving
        BaseState(BaseState&& other) noexcept = default;
        BaseState& operator=(BaseState&& other) noexcept = default;

        // Prevent copying
        BaseState(const BaseState&) = delete;
        BaseState& operator=(const BaseState&) = delete;

        explicit BaseState(StateEnum initialState) : currentState(initialState)
        {
        }
    };

    enum class PartyMemberStateEnum
    {
        Default,
        FollowingLeader,
        WaitingForLeader,
        DestinationUnreachable
    };

    class PartyMemberState : public BaseState<PartyMemberState, PartyMemberStateEnum>
    {
      public:
        std::unique_ptr<Connection> onLeaderMoveForwardCnx{};
        Event<entt::entity, entt::entity> onLeaderMove; // self, leader

        PartyMemberState() : BaseState(PartyMemberStateEnum::Default)
        {
        }
    };

    enum class PlayerStateEnum
    {
        Default,
        MovingToLocation,
        MovingToAttackEnemy,
        MovingToTalkToNPC,
        InDialog,
        DestinationUnreachable,
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
        Wave, // TODO: Can remove this now
        Combat
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