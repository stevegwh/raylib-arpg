//
// Created by Steve Wheeler on 30/12/2025.
//

#pragma once
#include "engine/components/BaseStateComponent.hpp"

namespace lq
{

    enum class PartyMemberStateEnum
    {
        Default,
        FollowingLeader,
        WaitingForLeader,
        DestinationUnreachable
    };

    class PartyMemberState : public sage::BaseStateComponent<PartyMemberState, PartyMemberStateEnum>
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

    class PlayerState : public sage::BaseStateComponent<PlayerState, PlayerStateEnum>
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

    class GameState : public sage::BaseStateComponent<GameState, GameStateEnum>
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

    class WavemobState : public sage::BaseStateComponent<WavemobState, WavemobStateEnum>
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

    class AbilityState : public sage::BaseStateComponent<AbilityState, AbilityStateEnum>
    {
    public:
        AbilityState() : BaseStateComponent(AbilityStateEnum::IDLE)
        {
        }
    };

} // namespace lq
