//
// Created by Steve Wheeler on 27/03/2024.
//

#include "ExampleScene.hpp"
#include "../GameObjectFactory.hpp"

#include "raylib.h"

namespace sage
{
void ExampleScene::Update()
{
    Scene::Update();
    data->transformSystem->Update();
    data->animationSystem->Update();
    data->dialogueSystem->Update();
    data->healthBarSystem->Update();
    data->combatSystem->Update();
}

void ExampleScene::Draw2D()
{
    data->dialogueSystem->Draw2D();
    Scene::Draw2D();
}

void ExampleScene::Draw3D()
{
    data->healthBarSystem->Draw3D();
    data->combatSystem->Draw3D();
    Scene::Draw3D();
}

ExampleScene::~ExampleScene()
{
    
}

ExampleScene::ExampleScene(entt::registry* _registry, std::unique_ptr<GameData> _data) :
Scene(_registry, std::move(_data))
{
    lightSubSystem->lights[0] = CreateLight(LIGHT_POINT, { 0, 25, 0 }, Vector3Zero(), WHITE, lightSubSystem->shader);
    auto playerId = GameObjectFactory::createPlayer(registry, data.get(), {20.0f, 0, 20.0f}, "Player");
    
    auto knight = GameObjectFactory::createKnight(registry, data.get(), {0.0f, 0, 20.0f}, "Knight");
    auto enemy = GameObjectFactory::createEnemy(registry, data.get(), {0.0f, 0, 30.0f}, "Enemy");

    data->Load();
    
    BoundingBox bb = {
        .min = { -1000.0f, 0.1f, -1000.0f },
        .max = {  1000.0f, 0.1f,  1000.0f }
    };
    GameObjectFactory::createFloor(registry, this, bb);
    //GameObjectFactory::loadBlenderLevel(registry, this);

    data->navigationGridSystem->Init(1000, 1.0f);
    data->navigationGridSystem->PopulateGrid();
}

} // sage