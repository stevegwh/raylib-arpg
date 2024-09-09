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
        entt::entity gameEntity;
        Timer timer{};

        void OnTimerEnd()
        {
            auto& gameState = registry->get<GameState>(gameEntity);
            gameState.ChangeState(gameEntity, GameStateEnum::Wave);
        }

      public:
        void Update(entt::entity entity) override
        {
            timer.Update(GetFrameTime());
            if (timer.HasFinished())
            {
                OnTimerEnd();
            }
        }

        void Draw3D(entt::entity entity) override
        {
        }

        void OnStateEnter(entt::entity entity) override
        {
            timer.Start();
        }

        void OnStateExit(entt::entity entity) override
        {
            timer.Stop();
        }

        DefaultState(entt::registry* _registry, GameData* _gameData, entt::entity _gameEntity)
            : StateMachine(_registry, _gameData), gameEntity(_gameEntity)
        {
            timer.SetMaxTime(2.0f);
        }
    };

    // ----------------------------

    class GameModeStateController::WaveState : public StateMachine
    {

        void initWave()
        {
            GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 10.0f}, "Enemy");
            GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 20.0f}, "Enemy");
            GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 30.0f}, "Enemy");
            GameObjectFactory::createEnemy(registry, gameData, {52.0f, 0, 40.0f}, "Enemy");
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

        WaveState(entt::registry* _registry, GameData* _gameData, entt::entity _gameEntity)
            : StateMachine(_registry, _gameData)
        {
            // Preload model(s)
            ResourceManager::GetInstance().EmplaceModel("resources/models/gltf/goblin.glb");
        }
    };

    // ----------------------------

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
        states[GameStateEnum::Default] = std::make_unique<DefaultState>(registry, _gameData, gameEntity);
        states[GameStateEnum::Wave] = std::make_unique<WaveState>(registry, _gameData, gameEntity);
        registry->emplace<GameState>(gameEntity);
    }
} // namespace sage