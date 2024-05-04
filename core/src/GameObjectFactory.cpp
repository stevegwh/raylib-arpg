//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"
#include "GameManager.hpp"
#include "scenes/Scene.hpp"
#include "components/Transform.hpp"
#include "components/Renderable.hpp"
#include "components/Collideable.hpp"
#include "components/WorldObject.hpp"
#include "components/Animation.hpp"

#include "raymath.h"

namespace sage
{
BoundingBox createRectangularBoundingBox(float length, float height) 
{
    BoundingBox bb;
    // Calculate half dimensions
    float halfLength = length / 2.0f;
    float halfHeight = height / 2.0f;

    // Set minimum bounds
    bb.min.x = -halfLength;
    bb.min.y = 0.0f;
    bb.min.z = -halfLength;

    // Set maximum bounds
    bb.max.x = halfLength;
    bb.max.y = height;
    bb.max.z = halfLength;

    return bb;
}

entt::entity GameObjectFactory::createPlayer(entt::registry* registry, ECSManager* ecs, Vector3 position, const char* name) 
{
    entt::entity id = registry->create();
    const char* modelPath = "resources/models/gltf/hero.glb";
    //sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"), std::string("resources/models/obj/cube_diffuse.png") };

    auto& transform = registry->emplace<Transform>(id);
    transform.position = position;
    transform.scale = 1.0f;
    transform.rotation = { 0, 0, 0 };
    
    auto model = LoadModel(modelPath);
    
    // Set animation hooks
    auto& animation = registry->emplace<Animation>(id, modelPath, &model);
    transform.dOnFinishMovement.connect<[](Animation& animation, entt::entity entity) {
        animation.ChangeAnimation(0);
    }>(animation);
    transform.dOnStartMovement.connect<[](Animation& animation, entt::entity entity) {
        animation.ChangeAnimation(3);
    }>(animation);
    ecs->userInput->dOnCreateKeyPressedEvent.connect<[](Animation& animation) {
        if (animation.animIndex == 1)
        {
            animation.ChangeAnimation(0);
        }
        else if (animation.animIndex == 0)
        {
            animation.ChangeAnimation(1);
        }
    }>(animation);
    
    Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
    auto& renderable = registry->emplace<Renderable>(id, model,std::string(modelPath), modelTransform);
    renderable.name = name;
    
    BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
    auto& collideable = registry->emplace<Collideable>(id, bb);
    collideable.collisionLayer = PLAYER;
    ecs->collisionSystem->UpdateWorldBoundingBox(id, ecs->transformSystem->GetMatrix(id));

    transform.dOnPositionUpdate.connect<[](Collideable& collideable, entt::entity entity) {
        collideable.onTransformUpdate(entity);
    }>(collideable);
    
    
    //ECS->collisionSystem->TransformUpdateSubscribe(id);

    auto& worldObject = registry->emplace<WorldObject>(id);
    return id;
}

void GameObjectFactory::createTower(entt::registry* registry, ECSManager* ecs, Vector3 position, const char* name) 
{
    auto id = registry->create();
    auto& transform = registry->emplace<Transform>(id);
    transform.position = position;
    transform.scale = 1.0f;
    
    sage::Material mat = { LoadTexture("resources/models/obj/turret_diffuse.png"), "resources/models/obj/turret_diffuse.png" };
    Model model = LoadModel("resources/models/obj/turret.obj");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
    auto& renderable = registry->emplace<Renderable>(id, model, mat, "resources/models/obj/turret.obj", MatrixIdentity());
    renderable.name = name;
    auto& collideable = registry->emplace<Collideable>(id, registry->get<Renderable>(id).CalculateModelBoundingBox());
    collideable.collisionLayer = BUILDING;
    ecs->collisionSystem->UpdateWorldBoundingBox(id, ecs->transformSystem->GetMatrix(id));
    registry->emplace<WorldObject>(id);
}

void GameObjectFactory::loadBlenderLevel(entt::registry* registry, Scene* scene)
{
    entt::entity id = registry->create();
//    sage::Material mat = { LoadTexture("resources/models/obj/PolyAdventureTexture_01.png"), "resources/models/obj/PolyAdventureTexture_01.png" };
//    const char* modelPath = "resources/models/obj/SM_Env_Rock_010.obj";
    const char* modelPath = "resources/models/gltf/tavern.glb";
    Model model = LoadModel(modelPath);
    //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
    
    auto& transform = registry->emplace<Transform>(id);
    transform.position = {0, 0, 0};
    transform.scale = 1.0f;

    Matrix modelTransform = MatrixMultiply(MatrixScale(7.0f, 7.0f, 7.0f), MatrixTranslate(0, 6.3f, 0));
    //Matrix modelTransform = MatrixScale(0.1f, 0.1f, 0.1f);
    auto& renderable = registry->emplace<Renderable>(id, model, std::string(modelPath), modelTransform);
    renderable.name = "Level";
    scene->lightSubSystem->LinkRenderableToLight(&renderable);
    
    // Create floor
    entt::entity floor = registry->create();
    BoundingBox modelBB = GetModelBoundingBox(renderable.model);
    Vector3 g0 = (Vector3){ modelBB.min.x, 0.1f, modelBB.min.z };
    Vector3 g2 = (Vector3){  modelBB.max.x, 0.1f,  modelBB.max.z };
    BoundingBox bb = {
        .min = g0,
        .max = g2
    };
    auto& floorCollidable = registry->emplace<Collideable>(floor, bb);
    floorCollidable.collisionLayer = FLOOR;
    
}
} // sage