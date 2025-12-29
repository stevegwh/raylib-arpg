#pragma once

#include "raylib.h"

#include "entt/entt.hpp"
#include "Event.hpp"
#include <vector>

namespace sage
{
    template <typename StateComponentType, typename StateEnum>
    class BaseStateComponent
    {
        StateEnum currentState;
        std::vector<Subscription> currentStateSubscriptions;

      public:
        // All subscriptions added here will be removed when ChangeState is called (via StateMachine).
        void ManageSubscription(const Subscription& newConnection)
        {
            currentStateSubscriptions.push_back(newConnection);
        }

        void RemoveAllSubscriptions()
        {
            for (auto& connection : currentStateSubscriptions)
            {
                connection.UnSubscribe();
            }
            currentStateSubscriptions.clear();
        }

        void SetStateEnum(StateEnum newState)
        {
            currentState = newState;
        }

        [[nodiscard]] StateEnum GetCurrentStateEnum() const
        {
            return currentState;
        }

        ~BaseStateComponent()
        {
            RemoveAllSubscriptions();
        }

        // Allow moving
        BaseStateComponent(BaseStateComponent&& other) noexcept = default;
        BaseStateComponent& operator=(BaseStateComponent&& other) noexcept = default;

        // Prevent copying
        BaseStateComponent(const BaseStateComponent&) = delete;
        BaseStateComponent& operator=(const BaseStateComponent&) = delete;

        explicit BaseStateComponent(StateEnum initialState) : currentState(initialState)
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

    class PartyMemberState : public BaseStateComponent<PartyMemberState, PartyMemberStateEnum>
    {
      public:
        PartyMemberState() : BaseStateComponent(PartyMemberStateEnum::Default)
        {
        }
    };

    enum class PlayerStateEnum
    {
        Default,
        MovingToLocation,
        MovingToAttackEnemy,
        MovingToTalkToNPC,
        MovingToLoot,
        InDialog,
        DestinationUnreachable,
        Combat
    };

    class PlayerState : public BaseStateComponent<PlayerState, PlayerStateEnum>
    {
      public:
        PlayerState() : BaseStateComponent(PlayerStateEnum::Default)
        {
        }
    };

    enum class GameStateEnum
    {
        Default,
        Wave, // TODO: Can remove this now
        Combat
    };

    class GameState : public BaseStateComponent<GameState, GameStateEnum>
    {
      public:
        GameState() : BaseStateComponent(GameStateEnum::Default)
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

    class WavemobState : public BaseStateComponent<WavemobState, WavemobStateEnum>
    {
      public:
        WavemobState() : BaseStateComponent(WavemobStateEnum::Default)
        {
        }
    };

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    class AbilityState : public BaseStateComponent<AbilityState, AbilityStateEnum>
    {
      public:
        AbilityState() : BaseStateComponent(AbilityStateEnum::IDLE)
        {
        }
    };

} // namespace sage