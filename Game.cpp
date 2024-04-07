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
Game::Game(UserInput* _cursor) : cursor(_cursor), eventManager(std::make_unique<EventManager>())
{
    auto playerId = GameObjectFactory::createPlayer({20.0f, 0, 20.0f}, "Player");
    ECS->actorMovementSystem->SetControlledActor(playerId);

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
    ECS->collisionSystem->AddComponent(std::move(floorCollidable));

//        auto floorWorldObject = std::make_unique<WorldObject>(floor);
//        worldSystem->AddComponent(std::move(floorWorldObject));

    //ECS->DeserializeMap(); // TODO: Should specify path to saved map of scene
    GameObjectFactory::loadBlenderLevel();
    
    // This should also be based on scene parameters
    ECS->navigationGridSystem->Init(100, 1.0f);
    ECS->navigationGridSystem->PopulateGrid();
    
    eventManager->Subscribe( [p = this] { p->onEditorModePressed(); }, *cursor->OnRunModePressedEvent);
}

Game::~Game()
{
}

void Game::Update()
{
    ECS->transformSystem->Update();
    ECS->animationSystem->Update();
    ECS->renderSystem->Update();
}

void Game::Draw3D()
{
    ECS->renderSystem->Draw();

    DrawGrid(100, 1.0f);

    for (const auto& gridSquareRow : ECS->navigationGridSystem->GetGridSquares())
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

void Game::onEditorModePressed()
{
    GM.SetStateEditor();
}

} // sage