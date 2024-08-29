//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"
#include "ResourceManager.hpp"
#include "scenes/Scene.hpp"

#include "EntityReflectionSignalRouter.hpp"
#include "GameData.hpp"
#include "UserInput.hpp"
#include <Timer.hpp>

#include "abilities/Abilities.hpp"

#include "components/Animation.hpp"
#include "components/Collideable.hpp"
#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/Dialogue.hpp"
#include "components/HealthBar.hpp"
#include "components/MovableActor.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "components/States.hpp"

#include "systems/AbilitySystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/LightSubSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/states/WavemobStateMachine.hpp"

#include "raymath.h"
#include <slib.hpp>

namespace sage
{
    BoundingBox createRectangularBoundingBox(float length, float height)
    {
        BoundingBox bb;
        // Calculate half dimensions
        float halfLength = length / 2.0f;
        // float halfHeight = height / 2.0f;

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

    entt::entity GameObjectFactory::createEnemy(
        entt::registry* registry, GameData* data, Vector3 position, const char* name)
    {
        entt::entity id = registry->create();
        // Model/Rendering
        auto modelPath = "resources/models/gltf/goblin.glb";
        // sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"),
        // std::string("resources/models/obj/cube_diffuse.png") };

        auto& transform = registry->emplace<sgTransform>(id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z}, id);
        transform.SetScale(1.0f, id);
        transform.SetRotation({0, 0, 0}, id);
        transform.movementSpeed = 0.05f;
        registry->emplace<MoveableActor>(id);

        // auto model = LoadModel(modelPath);
        auto model = ResourceManager::DynamicModelLoad(modelPath);

        auto& animation = registry->emplace<Animation>(id, modelPath, &model);
        animation.animationMap[AnimationEnum::IDLE] = 0;
        animation.animationMap[AnimationEnum::DEATH] = 0;
        animation.animationMap[AnimationEnum::MOVE] = 3;
        animation.animationMap[AnimationEnum::AUTOATTACK] = 1;
        animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

        Matrix modelTransform = MatrixScale(0.03f, 0.03f, 0.03f);
        auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
        renderable.name = name;
        // ---

        // Combat
        auto& combatable = registry->emplace<CombatableActor>(id);
        combatable.actorType = CombatableActorType::WAVEMOB;
        data->abilitySystem->RegisterAbility(id, AbilityEnum::ENEMY_AUTOATTACK);

        auto& healthbar = registry->emplace<HealthBar>(id);
        // ---

        // Collision
        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f);
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        // collideable.debugDraw = true;
        collideable.collisionLayer = CollisionLayer::ENEMY;
        // ---

        data->lightSubSystem->LinkRenderableToLight(&renderable);
        registry->emplace<WavemobState>(id);
        // Always set state last to ensure everything is initialised properly before.
        return id;
    }

    entt::entity GameObjectFactory::createKnight(
        entt::registry* registry, GameData* data, Vector3 position, const char* name)
    {
        entt::entity id = registry->create();
        auto modelPath = "resources/models/gltf/arissa.glb";
        // sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"),
        // std::string("resources/models/obj/cube_diffuse.png") };

        auto& transform = registry->emplace<sgTransform>(id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z}, id);
        transform.SetScale(1.0f, id);
        transform.SetRotation({0, 0, 0}, id);

        auto model = LoadModel(modelPath);
        auto& animation = registry->emplace<Animation>(id, modelPath, &model);
        animation.ChangeAnimation(0);

        Matrix modelTransform = MatrixScale(0.045f, 0.045f, 0.045f);
        auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
        renderable.name = name;
        renderable.name = name;

        // auto& combat = registry->emplace<HealthBar>(id);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::NPC;
        // collideable.SetWorldBoundingBox(transform.GetMatrix()); // TODO: Likely
        // redundant
        {
            entt::sink sink{transform.onPositionUpdate};
            sink.connect<&Collideable::OnTransformUpdate>(collideable);
        }

        auto& dialogue = registry->emplace<Dialogue>(id);
        dialogue.sentence = "Hello, this is a test sentence.";
        dialogue.conversationPos =
            Vector3Add(transform.position(), Vector3Multiply(transform.forward(), {10.0f, 1, 10.0f}));

        data->lightSubSystem->LinkRenderableToLight(&renderable);
        return id;
    }

    entt::entity GameObjectFactory::createPlayer(
        entt::registry* registry, GameData* data, Vector3 position, const char* name)
    {
        // TODO: On load, the actor's collision box doesn't seem to be correct. Causes a
        // bug that, if I don't move before casting a move, the enemies don't register
        // that the player has a collision box.
        entt::entity id = registry->create();
        auto modelPath = "resources/models/gltf/hero2.glb";

        auto& transform = registry->emplace<sgTransform>(id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z}, id);
        transform.SetScale(1.0f, id);
        transform.SetRotation({0, 0, 0}, id);

        auto& moveableActor = registry->emplace<MoveableActor>(id);

        auto model = LoadModel(modelPath);

        // Set animation hooks
        auto& animation = registry->emplace<Animation>(id, modelPath, &model);

        animation.animationMap[AnimationEnum::IDLE] = 2;
        animation.animationMap[AnimationEnum::MOVE] = 5;
        animation.animationMap[AnimationEnum::TALK] = 3;
        animation.animationMap[AnimationEnum::AUTOATTACK] = 1;
        animation.animationMap[AnimationEnum::SPIN] = 6;
        animation.ChangeAnimationByEnum(AnimationEnum::IDLE);

        {
            entt::sink sink{moveableActor.onFinishMovement};
            sink.connect<[](Animation& animation, entt::entity entity) {
                animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
            }>(animation);
        }
        {
            entt::sink sink{moveableActor.onStartMovement};
            sink.connect<[](Animation& animation, entt::entity entity) {
                animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
            }>(animation);
        }
        {
            // TODO: Just to test animations on demand
            entt::sink sink{data->userInput->keyIPressed};
            sink.connect<[](Animation& animation) {
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

        auto& controllable = registry->emplace<ControllableActor>(id, id, data->cursor.get());
        data->controllableActorSystem->SetControlledActor(id);

        // Combat
        auto& combatable = registry->emplace<CombatableActor>(id);
        combatable.actorType = CombatableActorType::PLAYER;

        // Initialise starting abilities
        data->abilitySystem->RegisterAbility(id, AbilityEnum::WHIRLWIND);
        data->abilitySystem->RegisterAbility(id, AbilityEnum::RAINFOFIRE);
        data->playerAbilitySystem->SetSlot(0, AbilityEnum::WHIRLWIND);
        data->playerAbilitySystem->SetSlot(1, AbilityEnum::RAINFOFIRE);
        data->abilitySystem->RegisterAbility(id, AbilityEnum::PLAYER_AUTOATTACK);

        data->signalReflectionManager->CreateHook<entt::entity>(
            id, data->cursor->onFloorClick, combatable.onAttackCancelled);

        // ---

        Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
        auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
        renderable.name = name;

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::PLAYER;

        data->lightSubSystem->LinkRenderableToLight(&renderable);

        registry->emplace<PlayerState>(id);
        // Always set state last to ensure everything is initialised properly before.
        return id;
    }

    void GameObjectFactory::createBuilding(
        entt::registry* registry,
        GameData* data,
        Vector3 position,
        const char* name,
        const char* modelPath,
        const char* texturePath)
    {
        auto id = registry->create();
        auto& transform = registry->emplace<sgTransform>(id);
        transform.SetPosition(position, id);
        transform.SetScale(2.0f, id);
        transform.SetRotation({0, 0, 0}, id);
        auto model = LoadModel(modelPath);
        MaterialPaths matPaths;
        matPaths.diffuse = texturePath;
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
            LoadTextureFromImage(ResourceManager::ImageLoad(matPaths.diffuse));
        auto& renderable = registry->emplace<Renderable>(id, model, matPaths, MatrixIdentity());
        renderable.name = name;
        auto& collideable = registry->emplace<Collideable>(id, CalculateModelBoundingBox(renderable.model));
        collideable.collisionLayer = CollisionLayer::BUILDING;
    }

    BoundingBox calculateFloorSize(const std::vector<Collideable*>& floorMeshes)
    {
        // TODO: Below doesn't seem to work always, depending on the map.
        BoundingBox mapBB{Vector3{0, 0, 0}, Vector3{0, 0, 0}}; // min, max
        for (const auto& col : floorMeshes)
        {
            if (col->worldBoundingBox.min.x <= mapBB.min.x && col->worldBoundingBox.min.z <= mapBB.min.z)
            {
                mapBB.min = col->worldBoundingBox.min;
            }
            if (col->worldBoundingBox.max.x >= mapBB.max.x && col->worldBoundingBox.max.z >= mapBB.max.z)
            {
                mapBB.max = col->worldBoundingBox.max;
            }
        }
        mapBB.min.y = 0.1f;
        mapBB.max.y = 0.1f;
        return mapBB;
    }

    void GameObjectFactory::loadMap(
        entt::registry* registry, Scene* scene, float& slices, const std::string& _mapPath)
    {

        MaterialPaths matPaths;
        matPaths.diffuse = "resources/models/obj/PolyAdventureTexture_01.png";
        Model parent = LoadModel(_mapPath.c_str());
        std::vector<Collideable*> floorMeshes;

        for (int i = 0; i < parent.meshCount; ++i)
        {
            entt::entity id = registry->create();
            auto& transform = registry->emplace<sgTransform>(id);
            transform.SetPosition({0, 0, 0}, id);
            transform.SetScale(1.0f, id);
            transform.SetRotation({0, 0, 0}, id);

            Matrix modelTransform = MatrixScale(5.0f, 5.0f, 5.0f);
            Model model = LoadModelFromMesh(parent.meshes[i]);
            // Matrix modelTransform = MatrixScale(1,1,1);
            auto& renderable = registry->emplace<Renderable>(id, model, matPaths, modelTransform);
            renderable.name = parent.meshes[i].name;
            renderable.serializable = false;

            scene->lightSubSystem->LinkRenderableToLight(&renderable);
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
                LoadTextureFromImage(ResourceManager::ImageLoad(matPaths.diffuse));
            auto& collideable = registry->emplace<Collideable>(id, CalculateModelBoundingBox(renderable.model));
            collideable.SetWorldBoundingBox(transform.GetMatrix());

            if (renderable.name.find("SM_Bld") != std::string::npos)
            {
                collideable.collisionLayer = CollisionLayer::BUILDING;
            }
            else if (renderable.name.find("SM_Env_NoWalk") != std::string::npos)
            {
                collideable.collisionLayer = CollisionLayer::TERRAIN;
            }
            else if (renderable.name.find("SM_Env") != std::string::npos)
            {
                collideable.collisionLayer = CollisionLayer::FLOOR;
                floorMeshes.push_back(&collideable);
            }
            else if (renderable.name.find("SM_Prop") != std::string::npos)
            {
                collideable.collisionLayer = CollisionLayer::BUILDING;
            }
            else
            {
                collideable.collisionLayer = CollisionLayer::DEFAULT;
            }
        }

        // Calculate grid based on walkable area
        BoundingBox mapBB{Vector3{-500, 0, -500}, Vector3{500, 0, 500}}; // min, max
        // BoundingBox mapBB = calculateFloorSize(floorMeshes);

        slices = mapBB.max.x - mapBB.min.x;
        // Create floor
        createFloor(registry, scene, mapBB);
    }

    void GameObjectFactory::createFloor(entt::registry* registry, Scene* scene, BoundingBox bb)
    {
        entt::entity floor = registry->create();
        auto& floorCollidable = registry->emplace<Collideable>(floor, bb);
        floorCollidable.collisionLayer = CollisionLayer::FLOOR;
    }

    void GameObjectFactory::createPortal(entt::registry* registry, GameData* data, Vector3 position)
    {
        {

            entt::entity id = registry->create();

            auto& transform = registry->emplace<sgTransform>(id);
            GridSquare actorIdx{};
            data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
            float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
            transform.SetPosition({position.x, 12, position.z}, id);
            transform.SetScale(1.0f, id);
            transform.SetRotation({0, 0, 0}, id);

            Texture2D texture = LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_50.png");
            Texture2D texture2 = LoadTexture("resources/textures/luos/Noise_Gradients/T_Random_45.png");

            Matrix modelTransform = MatrixRotateX(90 * DEG2RAD);
            Model model = LoadModelFromMesh(GenMeshPlane(20, 20, 1, 1));
            model.transform = modelTransform;

            auto& timer = registry->emplace<Timer>(id);
            timer.SetMaxTime(1000000);
            timer.Start();

            Shader shader = LoadShader(NULL, "resources/shaders/custom/portal.fs");
            int secondsLoc = GetShaderLocation(shader, "seconds");
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
            // Using MATERIAL_MAP_EMISSION as a spare slot to use for 2nd texture
            model.materials[0].maps[MATERIAL_MAP_EMISSION].texture = texture2;
            shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
            model.materials[0].shader = shader;

            auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
            renderable.name = "Portal";

            // Textures managed in renderable
            renderable.textures.push_back(texture);
            renderable.textures.push_back(texture2);

            renderable.shader = shader;
            renderable.reqShaderUpdate = [data, secondsLoc](entt::entity entity) -> void {
                auto& r = data->registry->get<Renderable>(entity);
                auto& t = data->registry->get<Timer>(entity);
                auto time = t.GetCurrentTime();
                SetShaderValue(r.shader.value(), secondsLoc, &time, SHADER_UNIFORM_FLOAT);
            };

            BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
            auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
            collideable.collisionLayer = CollisionLayer::BUILDING;
        }

        entt::entity id = registry->create();

        auto& transform = registry->emplace<sgTransform>(id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z}, id);
        transform.SetScale(10.0f, id);
        transform.SetRotation({0, 0, 0}, id);

        Matrix modelTransform = MatrixIdentity();
        Model model = ResourceManager::StaticModelLoad("resources/models/obj/portal.obj");

        model.transform = modelTransform;

        auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
        renderable.name = "Portal Outer";
        data->lightSubSystem->LinkRenderableToLight(&renderable);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::BUILDING;
    }

    void GameObjectFactory::createWizardTower(entt::registry* registry, GameData* data, Vector3 position)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sgTransform>(id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z}, id);
        transform.SetScale(1.0f, id);
        transform.SetRotation({0, 0, 0}, id);

        Matrix modelTransform = MatrixIdentity();
        Model model = ResourceManager::StaticModelLoad("resources/models/obj/Wizard Tower 1.obj");

        model.transform = modelTransform;

        auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
        renderable.name = "Wizard Tower";
        data->lightSubSystem->LinkRenderableToLight(&renderable);

        BoundingBox bb = CalculateModelBoundingBox(model);
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::BUILDING;

        registry->emplace<TowerState>(id);
    }
} // namespace sage
