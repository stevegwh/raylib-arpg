//
// Created by Steve Wheeler on 27/03/2024.
//

#include "Game.hpp"

#include "raylib.h"

#include <memory>

// Components
#include "../components/Collideable.hpp"

// Misc
#include "../GameManager.hpp"
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

    //DrawGrid(100, 1.0f);

//    for (const auto& gridSquareRow : ECS->navigationGridSystem->GetGridSquares())
//    {
//        for (const auto& gridSquare : gridSquareRow)
//        {
//            BoundingBox bb;
//            bb.min = gridSquare->worldPosMin;
//            bb.max = gridSquare->worldPosMax;
//            bb.max.y = 0.1f;
//            Color color = gridSquare->occupied ? RED : GREEN;
//            DrawBoundingBox(bb, color);
//        }
//    }
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

    //ECS->DeserializeMap(); // TODO: Should specify path to saved map of scene
    GameObjectFactory::loadBlenderLevel(registry, this);

    // This should also be based on scene parameters
    ecs->navigationGridSystem->Init(100, 1.0f);
    ecs->navigationGridSystem->PopulateGrid();
    
    userInput->dOnRunModePressedEvent.connect<[]() {
        //GM.SetState(2); // TODO: Find a way to do this
    }>();
    
}

} // sage