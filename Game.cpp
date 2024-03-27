//
// Created by Steve Wheeler on 27/03/2024.
//

#include "Game.hpp"

#include "raylib.h"

#include <memory>

// Components
#include "Collideable.hpp"

// Misc
#include "GameManager.hpp"
#include "GameObjectFactory.hpp"

namespace sage
{
Game::Game()
{
    auto playerId = GameObjectFactory::createPlayer({20.0f, 0, 20.0f}, "Player");
    GM.actorMovementSystem->SetControlledActor(playerId);

//        GameObjectFactory::createTower({0.0f, 0.0f, 0.0f}, "Tower");
//        GameObjectFactory::createTower({10.0f, 0.0f, 20.0f}, "Tower 2");
//
    // Ground quad
    EntityID floor = Registry::GetInstance().CreateEntity();
    Vector3 g0 = (Vector3){ -50.0f, 0.1f, -50.0f };
    Vector3 g2 = (Vector3){  50.0f, 0.1f,  50.0f };
    BoundingBox bb = {
        .min = g0,
        .max = g2
    };
    auto floorCollidable = std::make_unique<Collideable>(floor, bb);
    floorCollidable->collisionLayer = FLOOR;
    GM.collisionSystem->AddComponent(std::move(floorCollidable));

//        auto floorWorldObject = std::make_unique<WorldObject>(floor);
//        worldSystem->AddComponent(std::move(floorWorldObject));

    GM.DeserializeMap(); // TODO: Should specify path to saved map of scene
    // This should also be based on scene parameters
    GM.navigationGridSystem->Init(100, 1.0f);
    GM.navigationGridSystem->PopulateGrid();

}
Game::~Game()
{

}
void Game::Update()
{
    GM.transformSystem->Update();
}
void Game::Draw3D()
{
    GM.renderSystem->Draw();

    DrawGrid(100, 1.0f);

    for (const auto& gridSquareRow : GM.navigationGridSystem->GetGridSquares())
    {
        for (const auto& gridSquare : gridSquareRow)
        {
            BoundingBox bb;
            bb.min = gridSquare->worldPosMin;
            bb.max = gridSquare->worldPosMax;
            bb.max.y = 0.1f;
            Color color = gridSquare->occupied ? RED : GREEN;
            DrawBoundingBox(bb, color);
        }
    }
}
void Game::Draw2D()
{
    
}
} // sage