//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameModeStateMachine.hpp"

namespace lq
{
    // ====== Lifecycle ===============================================================

    void GameModeStateMachine::StartCombat()
    {
        ChangeState(GameCombatState{});
    }

    void GameModeStateMachine::Update()
    {
        auto& state = registry->get<GameState>(gameEntity);
        std::visit([this](auto& cur) { cur.Update(*this, gameEntity); }, state.current);
    }

    void GameModeStateMachine::Draw3D()
    {
    }

    GameModeStateMachine::GameModeStateMachine(entt::registry* _registry)
        : Base(_registry), gameEntity(_registry->create())
    {
        registry->emplace<GameState>(gameEntity);
    }
} // namespace lq
