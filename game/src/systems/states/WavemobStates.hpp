#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"

#include <variant>
#include <vector>

namespace lq
{
    class WavemobStateMachine;

    struct WavemobDefaultState
    {
        void OnEnter(WavemobStateMachine& machine, entt::entity entity);
        void OnExit(WavemobStateMachine& machine, entt::entity entity);
        void Update(WavemobStateMachine& machine, entt::entity entity);
    };

    struct WavemobTargetOutOfRangeState
    {
        void OnEnter(WavemobStateMachine& machine, entt::entity entity);
        void OnExit(WavemobStateMachine& machine, entt::entity entity);
        void Update(WavemobStateMachine& machine, entt::entity entity);
    };

    struct WavemobCombatState
    {
        void OnEnter(WavemobStateMachine& machine, entt::entity entity);
        void OnExit(WavemobStateMachine& machine, entt::entity entity);
        void Update(WavemobStateMachine& machine, entt::entity entity);
    };

    struct WavemobDyingState
    {
        void OnEnter(WavemobStateMachine& machine, entt::entity entity);
        void OnExit(WavemobStateMachine& machine, entt::entity entity);
        void Update(WavemobStateMachine& machine, entt::entity entity);
    };

    struct WavemobState
    {
        using Variant = std::variant<
            WavemobDefaultState,
            WavemobTargetOutOfRangeState,
            WavemobCombatState,
            WavemobDyingState>;

        Variant current = WavemobDefaultState{};
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

        ~WavemobState()
        {
            RemoveAllSubscriptions();
        }

        WavemobState() = default;
        WavemobState(WavemobState&& other) noexcept = default;
        WavemobState& operator=(WavemobState&& other) noexcept = default;
        WavemobState(const WavemobState&) = delete;
        WavemobState& operator=(const WavemobState&) = delete;
    };
} // namespace lq
