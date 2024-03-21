//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"

#include "raymath.h"

#include "Game.hpp"
#include "Registry.hpp"

#include "Transform.hpp"
#include "Renderable.hpp"
#include "Collideable.hpp"
#include "WorldObject.hpp"

namespace sage
{
    EntityID GameObjectFactory::createPlayer(Vector3 position, const char* name) 
    {
        EntityID id = Registry::GetInstance().CreateEntity();
        sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"), std::string("resources/models/obj/cube_diffuse.png") };
    
        auto transform = std::make_unique<Transform>(id);
        transform->position = position;
        transform->scale = 1.0f;
    
        auto renderable = std::make_unique<Renderable>(id, LoadModel("resources/models/obj/cube_steve.obj"), mat, std::string("resources/models/obj/cube_steve.obj"));
        renderable->name = name;
        
        auto collideable = std::make_unique<Collideable>(id, renderable->meshBoundingBox);
        collideable->worldBoundingBox.min = Vector3Add(collideable->worldBoundingBox.min, transform->position);
        collideable->worldBoundingBox.max = Vector3Add(collideable->worldBoundingBox.max, transform->position);
        collideable->collisionLayer = PLAYER;
    
        auto towerWorldObject1 = std::make_unique<WorldObject>(id);

        Game::GetInstance().renderSystem->AddComponent(std::move(renderable));
        Game::GetInstance().transformSystem->AddComponent(std::move(transform));
        Game::GetInstance().collisionSystem->AddComponent(std::move(collideable));
        Game::GetInstance().worldSystem->AddComponent(std::move(towerWorldObject1));
        return id;
    }

    void GameObjectFactory::createTower(Vector3 position, const char* name) 
    {
        EntityID newTowerId = Registry::GetInstance().CreateEntity();
        sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png"), "resources/models/obj/turret_diffuse.png" };
    
        auto towerRenderable1 = std::make_unique<Renderable>(newTowerId, LoadModel("resources/models/obj/turret.obj"), mat, "resources/models/obj/turret.obj");
        towerRenderable1->name = name;
    
        auto towerTransform1 = std::make_unique<Transform>(newTowerId);
        towerTransform1->position = position;
        towerTransform1->scale = 1.0f;
    
        auto towerCollidable1 = std::make_unique<Collideable>(newTowerId, towerRenderable1->meshBoundingBox);
        towerCollidable1->worldBoundingBox.min = Vector3Add(towerCollidable1->worldBoundingBox.min, towerTransform1->position);
        towerCollidable1->worldBoundingBox.max = Vector3Add(towerCollidable1->worldBoundingBox.max, towerTransform1->position);
        towerCollidable1->collisionLayer = BUILDING;
    
        auto towerWorldObject1 = std::make_unique<WorldObject>(newTowerId);

        Game::GetInstance().renderSystem->AddComponent(std::move(towerRenderable1));
        Game::GetInstance().transformSystem->AddComponent(std::move(towerTransform1));
        Game::GetInstance().collisionSystem->AddComponent(std::move(towerCollidable1));
        Game::GetInstance().worldSystem->AddComponent(std::move(towerWorldObject1));
    }
} // sage