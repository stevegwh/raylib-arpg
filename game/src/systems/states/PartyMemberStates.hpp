#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

#include <variant>
#include <vector>

namespace lq
{
    class PartyMemberStateMachine;

    struct PartyMemberDefaultState
    {
        void OnEnter(PartyMemberStateMachine& machine, entt::entity entity);
        void OnExit(PartyMemberStateMachine& machine, entt::entity entity);
        void Update(PartyMemberStateMachine& machine, entt::entity entity);
    };

    struct PartyMemberFollowingLeaderState
    {
        void OnEnter(PartyMemberStateMachine& machine, entt::entity entity);
        void OnExit(PartyMemberStateMachine& machine, entt::entity entity);
        void Update(PartyMemberStateMachine& machine, entt::entity entity);
    };

    struct PartyMemberWaitingForLeaderState
    {
        void OnEnter(PartyMemberStateMachine& machine, entt::entity entity);
        void OnExit(PartyMemberStateMachine& machine, entt::entity entity);
        void Update(PartyMemberStateMachine& machine, entt::entity entity);
    };

    struct PartyMemberDestinationUnreachableState
    {
        Vector3 originalDestination{};
        double timeStart{};
        unsigned int tryCount = 0;

        void OnEnter(PartyMemberStateMachine& machine, entt::entity entity);
        void OnExit(PartyMemberStateMachine& machine, entt::entity entity);
        void Update(PartyMemberStateMachine& machine, entt::entity entity);
    };

    struct PartyMemberState
    {
        using Variant = std::variant<
            PartyMemberDefaultState,
            PartyMemberFollowingLeaderState,
            PartyMemberWaitingForLeaderState,
            PartyMemberDestinationUnreachableState>;

        Variant current = PartyMemberDefaultState{};
        std::vector<sage::Subscription> subscriptions;

        void BindSubscription(sage::Subscription newSubscription)
        {
            subscriptions.push_back(std::move(newSubscription));
        }

        void RemoveAllSubscriptions()
        {
            for (auto& subscription : subscriptions)
            {
                subscription.UnSubscribe();
            }
            subscriptions.clear();
        }

        ~PartyMemberState()
        {
            RemoveAllSubscriptions();
        }

        PartyMemberState() = default;
        PartyMemberState(PartyMemberState&& other) noexcept = default;
        PartyMemberState& operator=(PartyMemberState&& other) noexcept = default;
        PartyMemberState(const PartyMemberState&) = delete;
        PartyMemberState& operator=(const PartyMemberState&) = delete;
    };
} // namespace lq
