#pragma once

#include "engine/Event.hpp"

#include "entt/entt.hpp"

#include <variant>
#include <vector>

namespace lq
{
    class GameModeStateMachine;

    struct GameDefaultState
    {
        void OnEnter(GameModeStateMachine& machine, entt::entity entity);
        void OnExit(GameModeStateMachine& machine, entt::entity entity);
        void Update(GameModeStateMachine& machine, entt::entity entity);
    };

    struct GameWaveState
    {
        void OnEnter(GameModeStateMachine& machine, entt::entity entity);
        void OnExit(GameModeStateMachine& machine, entt::entity entity);
        void Update(GameModeStateMachine& machine, entt::entity entity);
    };

    struct GameCombatState
    {
        void OnEnter(GameModeStateMachine& machine, entt::entity entity);
        void OnExit(GameModeStateMachine& machine, entt::entity entity);
        void Update(GameModeStateMachine& machine, entt::entity entity);
    };

    struct GameState
    {
        using Variant = std::variant<GameDefaultState, GameWaveState, GameCombatState>;

        Variant current = GameDefaultState{};
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

        ~GameState()
        {
            RemoveAllSubscriptions();
        }

        GameState() = default;
        GameState(GameState&& other) noexcept = default;
        GameState& operator=(GameState&& other) noexcept = default;
        GameState(const GameState&) = delete;
        GameState& operator=(const GameState&) = delete;
    };
} // namespace lq
