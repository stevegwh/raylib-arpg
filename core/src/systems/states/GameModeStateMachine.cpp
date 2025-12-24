//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameModeStateMachine.hpp"

#include "GameObjectFactory.hpp"
#include "Systems.hpp"
#include <ResourceManager.hpp>

#include <iostream>

#include "raylib.h"

namespace sage
{
    class GameModeStateMachine::DefaultState : public State
    {
        GameModeStateMachine* stateController;
        entt::entity gameEntity;

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity) override
        {
        }

        void OnExit(entt::entity entity) override
        {
        }

        DefaultState(
            entt::registry* _registry,
            Systems* _sys,
            entt::entity _gameEntity,
            GameModeStateMachine* _stateController)
            : State(_registry, _sys), gameEntity(_gameEntity), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class GameModeStateMachine::WaveState : public State
    {
        GameModeStateMachine* stateController;
        void initWave()
        {
        }

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity) override
        {
            // Create waves here (enemies etc)
            std::cout << "Wave state entered! \n";
            initWave();
        }

        void OnExit(entt::entity entity) override
        {
        }

        WaveState(
            entt::registry* _registry,
            Systems* _sys,
            entt::entity _gameEntity,
            GameModeStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
            // Preload model(s)
            // ResourceManager::GetInstance().EmplaceModel("resources/models/gltf/goblin.glb");
        }
    };

    // ----------------------------

    class GameModeStateMachine::CombatState : public State
    {
        GameModeStateMachine* stateController;

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnEnter(entt::entity entity) override
        {
            // Create waves here (enemies etc)
            std::cout << "Combat state entered! \n";
        }

        void OnExit(entt::entity entity) override
        {
        }

        CombatState(
            entt::registry* _registry,
            Systems* _sys,
            entt::entity _gameEntity,
            GameModeStateMachine* _stateController)
            : State(_registry, _sys), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void GameModeStateMachine::StartCombat()
    {
        ChangeState(gameEntity, GameStateEnum::Combat);
    }

    void GameModeStateMachine::Update()
    {
        const auto& state = registry->get<GameState>(gameEntity).GetCurrentState();
        GetState(state)->Update(gameEntity);
    }

    void GameModeStateMachine::Draw3D()
    {
        const auto& state = registry->get<GameState>(gameEntity).GetCurrentState();
        GetState(state)->Draw3D(gameEntity);
    }

    GameModeStateMachine::GameModeStateMachine(entt::registry* _registry, Systems* _sys)
        : StateMachine(_registry), gameEntity(_registry->create())
    {
        states[GameStateEnum::Default] = std::make_unique<DefaultState>(registry, _sys, gameEntity, this);
        states[GameStateEnum::Wave] = std::make_unique<WaveState>(registry, _sys, gameEntity, this);
        states[GameStateEnum::Combat] = std::make_unique<CombatState>(registry, _sys, gameEntity, this);
        registry->emplace<GameState>(gameEntity);
    }
} // namespace sage