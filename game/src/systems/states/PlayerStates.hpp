#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"

#include <variant>
#include <vector>

namespace lq
{
    class PlayerStateMachine;

    struct PlayerDefaultState
    {
        void OnEnter(PlayerStateMachine& machine, entt::entity entity);
        void OnExit(PlayerStateMachine& machine, entt::entity entity);
    };

    struct PlayerMovingToLocationState
    {
        void OnEnter(PlayerStateMachine& machine, entt::entity entity);
        void OnExit(PlayerStateMachine& machine, entt::entity entity);
    };

    struct PlayerMovingToAttackEnemyState
    {
        void OnEnter(PlayerStateMachine& machine, entt::entity entity);
        void OnExit(PlayerStateMachine& machine, entt::entity entity);
    };

    struct PlayerMovingToTalkState
    {
        entt::entity target = entt::null;

        void OnEnter(PlayerStateMachine& machine, entt::entity entity);
        void OnExit(PlayerStateMachine& machine, entt::entity entity);
    };

    struct PlayerMovingToLootState
    {
        entt::entity target = entt::null;

        void OnEnter(PlayerStateMachine& machine, entt::entity entity);
        void OnExit(PlayerStateMachine& machine, entt::entity entity);
    };

    struct PlayerInDialogState
    {
        entt::entity target = entt::null;

        void OnEnter(PlayerStateMachine& machine, entt::entity entity);
        void OnExit(PlayerStateMachine& machine, entt::entity entity);
    };

    struct PlayerCombatState
    {
        void OnEnter(PlayerStateMachine& machine, entt::entity entity);
        void OnExit(PlayerStateMachine& machine, entt::entity entity);
    };

    struct PlayerState
    {
        using Variant = std::variant<
            PlayerDefaultState,
            PlayerMovingToLocationState,
            PlayerMovingToAttackEnemyState,
            PlayerMovingToTalkState,
            PlayerMovingToLootState,
            PlayerInDialogState,
            PlayerCombatState>;

        Variant current = PlayerDefaultState{};
        std::vector<sage::Subscription> subscriptions;
        std::vector<sage::Subscription> persistentSubscriptions;

        void BindSubscription(sage::Subscription newSubscription)
        {
            subscriptions.push_back(std::move(newSubscription));
        }

        void BindPersistentSubscription(sage::Subscription newSubscription)
        {
            persistentSubscriptions.push_back(std::move(newSubscription));
        }

        void RemoveAllSubscriptions()
        {
            for (auto& subscription : subscriptions)
            {
                subscription.UnSubscribe();
            }
            subscriptions.clear();
        }

        void RemovePersistentSubscriptions()
        {
            for (auto& subscription : persistentSubscriptions)
            {
                subscription.UnSubscribe();
            }
            persistentSubscriptions.clear();
        }

        ~PlayerState()
        {
            RemoveAllSubscriptions();
            RemovePersistentSubscriptions();
        }

        PlayerState() = default;
        PlayerState(PlayerState&& other) noexcept = default;
        PlayerState& operator=(PlayerState&& other) noexcept = default;
        PlayerState(const PlayerState&) = delete;
        PlayerState& operator=(const PlayerState&) = delete;
    };
} // namespace lq
