//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameModeStateMachine.hpp"

#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include <ResourceManager.hpp>

#include <iostream>

#include "raylib.h"

namespace sage
{
    class GameModeStateController::DefaultState : public StateMachine
    {
        GameModeStateController* stateController;
        entt::entity gameEntity;

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
        }

        void OnStateExit(entt::entity entity) override
        {
        }

        DefaultState(
            entt::registry* _registry,
            GameData* _gameData,
            entt::entity _gameEntity,
            GameModeStateController* _stateController)
            : StateMachine(_registry, _gameData), gameEntity(_gameEntity), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    class GameModeStateController::WaveState : public StateMachine
    {
        GameModeStateController* stateController;
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

        void OnStateEnter(entt::entity entity) override
        {
            // Create waves here (enemies etc)
            std::cout << "Wave state entered! \n";
            initWave();
        }

        void OnStateExit(entt::entity entity) override
        {
        }

        WaveState(
            entt::registry* _registry,
            GameData* _gameData,
            entt::entity _gameEntity,
            GameModeStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
            // Preload model(s)
            // ResourceManager::GetInstance().EmplaceModel("resources/models/gltf/goblin.glb");
        }
    };

    // ----------------------------

    class GameModeStateController::CombatState : public StateMachine
    {
        GameModeStateController* stateController;

      public:
        void Update(entt::entity entity) override
        {
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
            // Create waves here (enemies etc)
            std::cout << "Combat state entered! \n";
        }

        void OnStateExit(entt::entity entity) override
        {
        }

        CombatState(
            entt::registry* _registry,
            GameData* _gameData,
            entt::entity _gameEntity,
            GameModeStateController* _stateController)
            : StateMachine(_registry, _gameData), stateController(_stateController)
        {
        }
    };

    // ----------------------------

    void GameModeStateController::StartCombat()
    {
        ChangeState(gameEntity, GameStateEnum::Combat);
    }

    void GameModeStateController::Update()
    {
        const auto& state = registry->get<GameState>(gameEntity).GetCurrentState();
        GetSystem(state)->Update(gameEntity);
    }

    void GameModeStateController::Draw3D()
    {
        const auto& state = registry->get<GameState>(gameEntity).GetCurrentState();
        GetSystem(state)->Draw3D(gameEntity);
    }

    GameModeStateController::GameModeStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry), gameEntity(_registry->create())
    {
        states[GameStateEnum::Default] = std::make_unique<DefaultState>(registry, _gameData, gameEntity, this);
        states[GameStateEnum::Wave] = std::make_unique<WaveState>(registry, _gameData, gameEntity, this);
        states[GameStateEnum::Combat] = std::make_unique<CombatState>(registry, _gameData, gameEntity, this);
        registry->emplace<GameState>(gameEntity);
    }
} // namespace sage