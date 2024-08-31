//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameStateMachine.hpp"

#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "systems/BaseSystem.hpp"
#include "systems/LightSubSystem.hpp"
#include <ResourceManager.hpp>

#include <iostream>

#include "raylib.h"

namespace sage
{
    class GameStateController::DefaultState : public StateMachine
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

        DefaultState(entt::registry* _registry, entt::entity _gameEntity)
            : StateMachine(_registry), gameEntity(_gameEntity)
        {
            timer.SetMaxTime(5.0f);
        }
    };

    // ----------------------------

    class GameStateController::WaveState : public StateMachine
    {
        GameData* gameData;
        void OnTimerEnd();

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
            : StateMachine(_registry), gameData(_gameData)
        {
            // Preload model(s)
            ResourceManager::DynamicModelLoad("resources/models/gltf/goblin.glb");
        }
    };

    // ----------------------------

    void GameStateController::Update()
    {
        const auto& state = registry->get<GameState>(gameEntity).GetCurrentState();
        GetSystem(state)->Update(gameEntity);
    }

    void GameStateController::Draw3D()
    {
        const auto& state = registry->get<GameState>(gameEntity).GetCurrentState();
        GetSystem(state)->Draw3D(gameEntity);
    }

    GameStateController::~GameStateController()
    {
        }

    GameStateController::GameStateController(entt::registry* _registry, GameData* _gameData)
        : StateMachineController(_registry), gameEntity(_registry->create())
    {

        states[GameStateEnum::Default] = std::make_unique<DefaultState>(_registry, gameEntity);
        states[GameStateEnum::Wave] = std::make_unique<WaveState>(_registry, _gameData, gameEntity);

        _registry->emplace<GameState>(gameEntity);
    }
} // namespace sage