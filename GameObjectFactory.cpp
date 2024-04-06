//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"

#include "raymath.h"

#include "GameManager.hpp"
#include "Registry.hpp"

#include "Transform.hpp"
#include "Renderable.hpp"
#include "Collideable.hpp"
#include "WorldObject.hpp"

namespace sage
{
    EntityID GameObjectFactory::createPlayer(Vector3 position, const char* name) 
    {
        EntityID id = Registry::GetInstance().CreateEntity(false);
        sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"), std::string("resources/models/obj/cube_diffuse.png") };
    
        auto transform = std::make_unique<Transform>(id);
        transform->position = position;
        transform->scale = 1.0f;
        transform->rotation = { 0, 0, 0 };
        Transform* const transform_ptr = transform.get();
        ECS->transformSystem->AddComponent(std::move(transform));

    
        //auto renderable = std::make_unique<Renderable>(id, LoadModel("resources/models/obj/cube_steve.obj"), mat, std::string("resources/models/obj/cube_steve.obj"));
        auto renderable = std::make_unique<Renderable>(id, LoadModel("resources/models/gltf/girl.glb"), std::string("resources/models/gltf/girl.glb"));
        renderable->name = name;
        renderable->anim = true;
        renderable->position = transform_ptr->position;
        renderable->scale = transform_ptr->scale;
        renderable->rotation = transform_ptr->rotation;
        
        auto collideable = std::make_unique<Collideable>(id, renderable->meshBoundingBox);
        collideable->collisionLayer = PLAYER;
        collideable->worldBoundingBox = renderable->meshBoundingBox;

        auto towerWorldObject1 = std::make_unique<WorldObject>(id);
        ECS->renderSystem->AddComponent(std::move(renderable), transform_ptr);
        ECS->collisionSystem->AddComponent(std::move(collideable));
        ECS->collisionSystem->UpdateWorldBoundingBox(id, ECS->transformSystem->GetMatrix(id));
        ECS->worldSystem->AddComponent(std::move(towerWorldObject1));
        return id;
    }

    void GameObjectFactory::createTower(Vector3 position, const char* name) 
    {
        EntityID newTowerId = Registry::GetInstance().CreateEntity();
        
        auto towerTransform1 = std::make_unique<Transform>(newTowerId);
        towerTransform1->position = position;
        towerTransform1->scale = 1.0f;
        
        sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png"), "resources/models/obj/turret_diffuse.png" };
        Model model = LoadModel("resources/models/obj/turret.obj");
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
        auto towerRenderable1 = std::make_unique<Renderable>(newTowerId, model, mat, "resources/models/obj/turret.obj");
        towerRenderable1->name = name;
    
        auto towerCollidable1 = std::make_unique<Collideable>(newTowerId, towerRenderable1->meshBoundingBox);
        towerCollidable1->worldBoundingBox.min = Vector3Add(towerCollidable1->worldBoundingBox.min, towerTransform1->position);
        towerCollidable1->worldBoundingBox.max = Vector3Add(towerCollidable1->worldBoundingBox.max, towerTransform1->position);
        towerCollidable1->collisionLayer = BUILDING;
    
        auto towerWorldObject1 = std::make_unique<WorldObject>(newTowerId);
        Transform* const transform_ptr = towerTransform1.get();
        ECS->transformSystem->AddComponent(std::move(towerTransform1));
        ECS->renderSystem->AddComponent(std::move(towerRenderable1), transform_ptr);
        ECS->collisionSystem->AddComponent(std::move(towerCollidable1));
        ECS->worldSystem->AddComponent(std::move(towerWorldObject1));
    }
} // sage