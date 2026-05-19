#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"

#include <variant>
#include <vector>

namespace lq
{
    class AbilityStateMachine;

    struct AbilityIdleState
    {
        void OnEnter(AbilityStateMachine& machine, entt::entity entity);
        void OnExit(AbilityStateMachine& machine, entt::entity entity);
        void Update(AbilityStateMachine& machine, entt::entity entity);
    };

    struct AbilityCursorSelectState
    {
        void OnEnter(AbilityStateMachine& machine, entt::entity entity);
        void OnExit(AbilityStateMachine& machine, entt::entity entity);
        void Update(AbilityStateMachine& machine, entt::entity entity);
    };

    struct AbilityAwaitingExecutionState
    {
        void OnEnter(AbilityStateMachine& machine, entt::entity entity);
        void OnExit(AbilityStateMachine& machine, entt::entity entity);
        void Update(AbilityStateMachine& machine, entt::entity entity);
    };

    struct AbilityState
    {
        using Variant =
            std::variant<AbilityIdleState, AbilityCursorSelectState, AbilityAwaitingExecutionState>;

        Variant current = AbilityIdleState{};
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

        ~AbilityState()
        {
            RemoveAllSubscriptions();
        }

        AbilityState() = default;
        AbilityState(AbilityState&& other) noexcept = default;
        AbilityState& operator=(AbilityState&& other) noexcept = default;
        AbilityState(const AbilityState&) = delete;
        AbilityState& operator=(const AbilityState&) = delete;
    };
} // namespace lq
