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
        Matrix modelTransform = MatrixMultiply(MatrixScale(0.035f, 0.035f, 0.035f) , MatrixRotateX(DEG2RAD*90));
        auto renderable = std::make_unique<Renderable>(id, 
                                                       LoadModel("resources/models/gltf/girl.glb"), 
                                                       std::string("resources/models/gltf/girl.glb"),
                                                       modelTransform);
        renderable->name = name;
        renderable->anim = true;
        renderable->position = transform_ptr->position;
        renderable->scale = transform_ptr->scale;
        renderable->rotation = transform_ptr->rotation;
        ECS->renderSystem->AddComponent(std::move(renderable), transform_ptr);
        
        auto collideable = std::make_unique<Collideable>(id, ECS->renderSystem->GetComponent(id)->CalculateModelBoundingBox());
        collideable->collisionLayer = PLAYER;

        auto towerWorldObject1 = std::make_unique<WorldObject>(id);

        ECS->collisionSystem->AddComponent(std::move(collideable));
        ECS->collisionSystem->UpdateWorldBoundingBox(id, ECS->transformSystem->GetMatrix(id));
        ECS->worldSystem->AddComponent(std::move(towerWorldObject1));
        return id;
    }

    void GameObjectFactory::createTower(Vector3 position, const char* name) 
    {
        EntityID id = Registry::GetInstance().CreateEntity();
        
        auto transform = std::make_unique<Transform>(id);
        transform->position = position;
        transform->scale = 1.0f;
        Transform* const transform_ptr = transform.get();
        ECS->transformSystem->AddComponent(std::move(transform));
        
        sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png"), "resources/models/obj/turret_diffuse.png" };
        Model model = LoadModel("resources/models/obj/turret.obj");
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
        auto renderable = std::make_unique<Renderable>(id, model, mat, "resources/models/obj/turret.obj", MatrixIdentity());
        renderable->name = name;
        ECS->renderSystem->AddComponent(std::move(renderable), transform_ptr);
        auto collideable = std::make_unique<Collideable>(id, ECS->renderSystem->GetComponent(id)->CalculateModelBoundingBox());
        collideable->collisionLayer = BUILDING;
        ECS->collisionSystem->AddComponent(std::move(collideable));
        ECS->collisionSystem->UpdateWorldBoundingBox(id, ECS->transformSystem->GetMatrix(id));
    
        auto worldObject = std::make_unique<WorldObject>(id);
        ECS->worldSystem->AddComponent(std::move(worldObject));
    }
} // sage