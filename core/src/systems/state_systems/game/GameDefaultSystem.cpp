//
// Created by Steve Wheeler on 31/07/2024.
//

#include "GameDefaultSystem.hpp"
#include "components/states/GameStates.hpp"

#include <iostream>
#include <tuple>

#include "raylib.h"

namespace sage
{

    void GameDefaultSystem::OnTimerEnd()
    {
        ChangeState<StateGameWaveattack, GameStates>(gameEntity);
    }

    void GameDefaultSystem::Update()
    {
        timer.Update(GetFrameTime());
        if (timer.HasFinished())
        {
            OnTimerEnd();
        }
    }

    void GameDefaultSystem::Draw3D()
    {
    }

    void GameDefaultSystem::OnStateEnter(entt::entity entity)
    {

        timer.Start();
    }

    void GameDefaultSystem::OnStateExit(entt::entity entity)
    {
        timer.Stop();
    }

    GameDefaultSystem::GameDefaultSystem(
        entt::registry* _registry, entt::entity _gameEntity)
        : StateMachine(_registry), gameEntity(_gameEntity)
    {
        timer.SetMaxTime(5.0f);
    }
} // namespace sage