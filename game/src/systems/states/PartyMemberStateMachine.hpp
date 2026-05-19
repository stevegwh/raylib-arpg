// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "PartyMemberStates.hpp"
#include "engine/systems/states/StateMachineBase.hpp"

#include "entt/entt.hpp"

namespace lq
{
    class Systems;

    class PartyMemberStateMachine final
        : public sage::StateMachineBase<PartyMemberStateMachine, PartyMemberState>
    {
        using Base = sage::StateMachineBase<PartyMemberStateMachine, PartyMemberState>;
        friend Base;
        friend struct PartyMemberDefaultState;
        friend struct PartyMemberFollowingLeaderState;
        friend struct PartyMemberWaitingForLeaderState;
        friend struct PartyMemberDestinationUnreachableState;

        Systems* sys;

        template <typename State>
        void onEnter(State& state, const entt::entity entity)
        {
            state.OnEnter(*this, entity);
        }

        template <typename State>
        void onExit(State& state, const entt::entity entity)
        {
            state.OnExit(*this, entity);
        }

        void onLeaderMove(entt::entity entity);
        void onFollowingTargetPathChanged(entt::entity entity, entt::entity target);

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity) const
        {
        }

      public:
        void Update();
        void Draw3D();

        ~PartyMemberStateMachine() = default;
        PartyMemberStateMachine(entt::registry* _registry, Systems* sys);
    };
} // namespace lq
