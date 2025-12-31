// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "engine/systems/states/StateMachine.hpp"
#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class PartyMemberStateMachine final : public sage::StateMachine<PartyMemberState, PartyMemberStateEnum>
    {
        Systems* sys;

        class DefaultState;
        class FollowingLeaderState;
        class WaitingForLeaderState;
        class DestinationUnreachableState;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity) const;

      public:
        void Update();
        void Draw3D();

        ~PartyMemberStateMachine() override = default;
        PartyMemberStateMachine(entt::registry* _registry, Systems* sys);

        friend class StateMachine; // Required for CRTP
        friend class PlayerStateMachine;
    };
} // namespace lq