#include "GameModeStates.hpp"

#include "GameModeStateMachine.hpp"

#include <iostream>

namespace lq
{
    void GameDefaultState::OnEnter(GameModeStateMachine&, entt::entity)
    {
    }

    void GameDefaultState::OnExit(GameModeStateMachine&, entt::entity)
    {
    }

    void GameDefaultState::Update(GameModeStateMachine&, entt::entity)
    {
    }

    void GameWaveState::OnEnter(GameModeStateMachine&, entt::entity)
    {
        std::cout << "Wave state entered! \n";
    }

    void GameWaveState::OnExit(GameModeStateMachine&, entt::entity)
    {
    }

    void GameWaveState::Update(GameModeStateMachine&, entt::entity)
    {
    }

    void GameCombatState::OnEnter(GameModeStateMachine&, entt::entity)
    {
        std::cout << "Combat state entered! \n";
    }

    void GameCombatState::OnExit(GameModeStateMachine&, entt::entity)
    {
    }

    void GameCombatState::Update(GameModeStateMachine&, entt::entity)
    {
    }
} // namespace lq
