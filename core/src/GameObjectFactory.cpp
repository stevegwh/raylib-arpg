//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"
#include "Application.hpp"
#include "scenes/Scene.hpp"
#include "components/Transform.hpp"
#include "components/Renderable.hpp"
#include "components/Collideable.hpp"
#include "components/WorldObject.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"
#include "components/CombatableActor.hpp"

#include "raymath.h"

namespace sage
{
BoundingBox createRectangularBoundingBox(float length, float height) 
{
    BoundingBox bb;
    // Calculate half dimensions
    float halfLength = length / 2.0f;
    //float halfHeight = height / 2.0f;

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

entt::entity GameObjectFactory::createEnemy(entt::registry* registry, GameData* game, Vector3 position, const char* name)
{
    entt::entity id = registry->create();
    const char* modelPath = "resources/models/gltf/goblin.glb";
    //sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"), std::string("resources/models/obj/cube_diffuse.png") };

    auto& transform = registry->emplace<Transform>(id);
    transform.position = position;
    transform.scale = 1.0f;
    transform.rotation = { 0, 0, 0 };

    auto model = LoadModel(modelPath);

    auto& animation = registry->emplace<Animation>(id, modelPath, &model);
    animation.animationMap[AnimationEnum::IDLE] = 0;
    animation.animationMap[AnimationEnum::DEATH] = 0;
    animation.animationMap[AnimationEnum::MOVE] = 3;
    animation.animationMap[AnimationEnum::AUTOATTACK] = 1;
    animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

    Matrix modelTransform = MatrixScale(0.045f, 0.045f, 0.045f);
    auto& renderable = registry->emplace<Renderable>(id, model,std::string(modelPath), modelTransform);
    renderable.name = name;

    auto& healthbar = registry->emplace<HealthBar>(id); // TODO: "HealthBar" should be separate from something like CombatData
    
    auto& combatable = registry->emplace<CombatableActor>(id);
    
    BoundingBox bb = renderable.CalculateModelBoundingBox();
    auto& collideable = registry->emplace<Collideable>(id, bb);
    collideable.collisionLayer = CollisionLayer::ENEMY;
    game->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
    {
        entt::sink sink{transform.onPositionUpdate};
        sink.connect<[](CollisionSystem& collisionSystem, entt::entity entity) {
            collisionSystem.OnTransformUpdate(entity);
        }>(*game->collisionSystem);
    }
    
    auto& worldObject = registry->emplace<WorldObject>(id);
    return id;
}

entt::entity GameObjectFactory::createKnight(entt::registry* registry, GameData* game, Vector3 position, const char* name)
{
    entt::entity id = registry->create();
    const char* modelPath = "resources/models/gltf/arissa.glb";
    //sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"), std::string("resources/models/obj/cube_diffuse.png") };

    auto& transform = registry->emplace<Transform>(id);
    transform.position = position;
    transform.scale = 1.0f;
    transform.rotation = { 0, 0, 0 };

    auto model = LoadModel(modelPath);
    auto& animation = registry->emplace<Animation>(id, modelPath, &model);
    animation.ChangeAnimation(0);

    Matrix modelTransform = MatrixScale(0.045f, 0.045f, 0.045f);
    auto& renderable = registry->emplace<Renderable>(id, model,std::string(modelPath), modelTransform);
    renderable.name = name;
    
    //auto& combat = registry->emplace<HealthBar>(id);

    BoundingBox bb = renderable.CalculateModelBoundingBox();
    auto& collideable = registry->emplace<Collideable>(id, bb);
    collideable.collisionLayer = CollisionLayer::NPC;
    game->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
    {
        entt::sink sink{transform.onPositionUpdate};
        sink.connect<[](CollisionSystem& collisionSystem, entt::entity entity) {
            collisionSystem.OnTransformUpdate(entity);
        }>(*game->collisionSystem);
    }

    auto& dialogue = registry->emplace<Dialogue>(id);
    dialogue.sentence = "Hello, this is a test sentence.";
    dialogue.conversationPos = Vector3Add(transform.position, Vector3Multiply(transform.forward(), { 10.0f, 1, 10.0f}));
    auto& worldObject = registry->emplace<WorldObject>(id);
    return id;
}

entt::entity GameObjectFactory::createPlayer(entt::registry* registry, GameData* game, Vector3 position, const char* name) 
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

    animation.animationMap[AnimationEnum::IDLE] = 2;
    animation.animationMap[AnimationEnum::MOVE] = 5;
    animation.animationMap[AnimationEnum::TALK] = 3;
    animation.animationMap[AnimationEnum::AUTOATTACK] = 1;
    animation.ChangeAnimationByEnum(AnimationEnum::IDLE);

    {
        entt::sink sink{transform.onFinishMovement};
        sink.connect<[](Animation& animation, entt::entity entity) {
            animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        }>(animation);
    }
    {
        entt::sink sink{transform.onStartMovement};
        sink.connect<[](Animation& animation, entt::entity entity) {
            animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
        }>(animation);
    }
    {
        entt::sink sink{game->userInput->keyIPressed};
        sink.connect<[](Animation& animation) { // TODO: Just to test animations on demand
            if (animation.animIndex == 0)
            {
                animation.ChangeAnimationByEnum(AnimationEnum::TALK);
            }
            else if (animation.animIndex == 2)
            {
                animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
            }
        }>(animation);
    }

    auto& combatable = registry->emplace<CombatableActor>(id);
    
    Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
    auto& renderable = registry->emplace<Renderable>(id, model,std::string(modelPath), modelTransform);
    renderable.name = name;
    
    BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
    auto& collideable = registry->emplace<Collideable>(id, bb);
    collideable.collisionLayer = CollisionLayer::PLAYER;
    game->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
    {
        entt::sink sink{transform.onPositionUpdate};
        sink.connect<&CollisionSystem::OnTransformUpdate>(*game->collisionSystem);
    }
    auto& worldObject = registry->emplace<WorldObject>(id);

    auto& actor = registry->emplace<ControllableActor>(id);
    actor.pathfindingBounds = 50;
    game->actorMovementSystem->SetControlledActor(id);
    
    return id;
}

void GameObjectFactory::createBuilding(entt::registry* registry, GameData* data, Vector3 position, const char* name,
                                       const char* modelPath, const char* texturePath) 
{
    auto id = registry->create();
    auto& transform = registry->emplace<Transform>(id);
    transform.position = position;
    transform.scale = 2.0f;
    sage::Material mat = { LoadTexture(texturePath), texturePath };
    Model model = LoadModel(modelPath);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mat.diffuse;
    auto& renderable = registry->emplace<Renderable>(id, model, mat, modelPath, MatrixIdentity());
    renderable.name = name;
    auto& collideable = registry->emplace<Collideable>(id, registry->get<Renderable>(id).CalculateModelBoundingBox());
    collideable.collisionLayer = CollisionLayer::BUILDING;
    data->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
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
    BoundingBox modelBB = GetModelBoundingBox(renderable.model);
    Vector3 g0 = { modelBB.min.x, 0.1f, modelBB.min.z };
    Vector3 g2 = { modelBB.max.x, 0.1f,  modelBB.max.z };
    BoundingBox bb = {
        .min = g0,
        .max = g2
    };
    createFloor(registry, scene, bb);
    
}

void GameObjectFactory::createFloor(entt::registry *registry, sage::Scene *scene, BoundingBox bb)
{
    entt::entity floor = registry->create();
    auto& floorCollidable = registry->emplace<Collideable>(floor, bb);
    floorCollidable.collisionLayer = CollisionLayer::FLOOR;
}
} // sage
