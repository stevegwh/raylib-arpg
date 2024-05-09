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
    data->renderSystem->Update();
}

void ExampleScene::Draw3D()
{
    data->renderSystem->Draw();
    Scene::Draw3D();
}

void ExampleScene::Draw2D()
{
    
}

ExampleScene::~ExampleScene()
{
    
}

ExampleScene::ExampleScene(entt::registry* _registry, std::unique_ptr<GameData> _data) :
Scene(_registry, std::move(_data))
{
    lightSubSystem->lights[0] = CreateLight(LIGHT_POINT, (Vector3){ 0, 25, 0 }, Vector3Zero(), WHITE, lightSubSystem->shader);
    auto playerId = GameObjectFactory::createPlayer(registry, data.get(), {20.0f, 0, 20.0f}, "Player");
    auto& actor = registry->emplace<Actor>(playerId);
    actor.pathfindingBounds = 50;
    data->actorMovementSystem->SetControlledActor(playerId);
    
    
    auto knight = GameObjectFactory::createKnight(registry, data.get(), {0.0f, 0, 20.0f}, "Knight");

    data->Load();
    
    BoundingBox bb = {
        .min = (Vector3){ -1000.0f, 0.1f, -1000.0f },
        .max = (Vector3){  1000.0f, 0.1f,  1000.0f }
    };
    GameObjectFactory::createFloor(registry, this, bb);
    //GameObjectFactory::loadBlenderLevel(registry, this);

    data->navigationGridSystem->Init(1000, 1.0f);
    data->navigationGridSystem->PopulateGrid();
}

} // sage