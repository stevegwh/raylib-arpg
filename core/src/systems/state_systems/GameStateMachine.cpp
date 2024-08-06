//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameStateMachine.hpp"

#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "systems/BaseSystem.hpp"
#include <ResourceManager.hpp>

#include <iostream>

#include "raylib.h"

namespace sage
{
    namespace gamestates
    {
        void DefaultState::OnTimerEnd()
        {
            ChangeState<StateGameWave, GameStates>(gameEntity);
        }

        void DefaultState::Update()
        {
            timer.Update(GetFrameTime());
            if (timer.HasFinished())
            {
                OnTimerEnd();
            }
        }

        void DefaultState::Draw3D()
        {
        }

        void DefaultState::OnStateEnter(entt::entity entity)
        {

            timer.Start();
        }

        void DefaultState::OnStateExit(entt::entity entity)
        {
            timer.Stop();
        }

        DefaultState::DefaultState(entt::registry* _registry, entt::entity _gameEntity)
            : StateMachine(_registry), gameEntity(_gameEntity)
        {
            timer.SetMaxTime(5.0f);
        }

        void WaveState::initWave()
        {
            auto enemy2 = GameObjectFactory::createEnemy(
                registry, gameData, {52.0f, 0, 10.0f}, "Enemy");
            auto enemy3 = GameObjectFactory::createEnemy(
                registry, gameData, {52.0f, 0, 20.0f}, "Enemy");
            auto enemy4 = GameObjectFactory::createEnemy(
                registry, gameData, {52.0f, 0, 30.0f}, "Enemy");
            auto enemy5 = GameObjectFactory::createEnemy(
                registry, gameData, {52.0f, 0, 40.0f}, "Enemy");
        }

        void WaveState::Update()
        {
        }

        void WaveState::Draw3D()
        {
        }

        void WaveState::OnStateEnter(entt::entity entity)
        {
            // Create waves here (enemies etc)
            std::cout << "Wave state entered! \n";
            initWave();
        }

        void WaveState::OnStateExit(entt::entity entity)
        {
        }

        WaveState::WaveState(
            entt::registry* _registry, GameData* _gameData, entt::entity _gameEntity)
            : StateMachine(_registry), gameData(_gameData)
        {
            // Preload model(s)
            ResourceManager::DynamicModelLoad("resources/models/gltf/goblin.glb");
        }
    } // namespace gamestates

    void GameStateController::Update()
    {
        for (auto& system : systems)
        {
            system->Update();
        }
    }
    GameStateController::GameStateController(
        entt::registry* _registry, GameData* _gameData)
    {
        gameEntity = _registry->create();
        defaultState = std::make_unique<gamestates::DefaultState>(_registry, gameEntity);
        waveState =
            std::make_unique<gamestates::WaveState>(_registry, _gameData, gameEntity);
        systems.push_back(defaultState.get());
        systems.push_back(waveState.get());
        _registry->emplace<StateGameDefault>(gameEntity);
    }
} // namespace sage