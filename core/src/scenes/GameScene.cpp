//
// Created by Steve Wheeler on 27/03/2024.
//

#include "GameScene.hpp"
#include "../GameObjectFactory.hpp"

#include "raylib.h"

namespace sage
{
void GameScene::Update()
{
    game->transformSystem->Update();
    game->animationSystem->Update();
    game->renderSystem->Update();
}

void GameScene::Draw3D()
{
    game->renderSystem->Draw();
}

void GameScene::Draw2D()
{
    
}

GameScene::~GameScene()
{
    
}

GameScene::GameScene(entt::registry* _registry, Game* _game, Settings _settings) :
Scene(_registry, _game, _settings)
{
    lightSubSystem->lights[0] = CreateLight(LIGHT_POINT, (Vector3){ 0, 25, 0 }, Vector3Zero(), WHITE, lightSubSystem->shader);
    auto playerId = GameObjectFactory::createPlayer(registry, _game, {20.0f, 0, 20.0f}, "Player");
    game->actorMovementSystem->SetControlledActor(playerId);
    
    game->Load();
    
    BoundingBox bb = {
        .min = (Vector3){ -1000.0f, 0.1f, -1000.0f },
        .max = (Vector3){  1000.0f, 0.1f,  1000.0f }
    };
    GameObjectFactory::createFloor(registry, this, bb);
    //GameObjectFactory::loadBlenderLevel(registry, this);
    
    game->navigationGridSystem->Init(1000, 1.0f);
    game->navigationGridSystem->PopulateGrid();
}

} // sage