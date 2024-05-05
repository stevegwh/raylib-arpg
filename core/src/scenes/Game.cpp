//
// Created by Steve Wheeler on 27/03/2024.
//

#include "Game.hpp"

#include "raylib.h"

#include <memory>

// Components

// Misc
#include "../GameObjectFactory.hpp"

namespace sage
{
void Game::Update()
{
    ecs->transformSystem->Update();
    ecs->animationSystem->Update();
    ecs->renderSystem->Update();
}

void Game::Draw3D()
{
    ecs->renderSystem->Draw();
}

void Game::Draw2D()
{
    
}

Game::~Game()
{
    
}

Game::Game(entt::registry* _registry, ECSManager* _ecs) : 
Scene(_registry, _ecs)
{
    lightSubSystem->lights[0] = CreateLight(LIGHT_POINT, (Vector3){ 0, 25, 0 }, Vector3Zero(), WHITE, lightSubSystem->shader);
    auto playerId = GameObjectFactory::createPlayer(registry, _ecs, {20.0f, 0, 20.0f}, "Player");
    ecs->actorMovementSystem->SetControlledActor(playerId);
    
    ecs->Load();
    BoundingBox bb = {
        .min = (Vector3){ -50.0f, 0.1f, -50.0f },
        .max = (Vector3){  50.0f, 0.1f,  50.0f }
    };
    GameObjectFactory::createFloor(registry, this, bb);
    //GameObjectFactory::loadBlenderLevel(registry, this);

    // This should also be based on scene parameters
    ecs->navigationGridSystem->Init(100, 1.0f);
    ecs->navigationGridSystem->PopulateGrid();
    
}

} // sage