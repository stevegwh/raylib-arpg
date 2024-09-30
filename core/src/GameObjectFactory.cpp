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

#include "components/Ability.hpp"
#include "components/Animation.hpp"
#include "components/Collideable.hpp"
#include "components/CombatableActor.hpp"
#include "components/ControllableActor.hpp"
#include "components/Dialogue.hpp"
#include "components/HealthBar.hpp"
#include "components/MoveableActor.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "components/States.hpp"

#include "AbilityFactory.hpp"
#include "AssetManager.hpp"
#include "systems/ActorMovementSystem.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/LightSubSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/states/WavemobStateMachine.hpp"

#include "components/Weapon.hpp"
#include "raymath.h"

#include <slib.hpp>

#include <iostream>

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

        auto& transform = registry->emplace<sgTransform>(id, id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z});

        auto& moveable = registry->emplace<MoveableActor>(id);
        moveable.movementSpeed = 0.25f;

        Matrix modelTransform = MatrixScale(0.03f, 0.03f, 0.03f);
        auto& renderable = registry->emplace<Renderable>(
            id, ResourceManager::GetInstance().GetModelDeepCopy(AssetID::MDL_ENEMY_GOBLIN), modelTransform);
        renderable.name = name;

        auto& animation = registry->emplace<Animation>(id, AssetID::MDL_ENEMY_GOBLIN);
        animation.animationMap[AnimationEnum::IDLE] = 1;
        animation.animationMap[AnimationEnum::DEATH] = 0;
        animation.animationMap[AnimationEnum::WALK] = 4;
        animation.animationMap[AnimationEnum::AUTOATTACK] = 2;
        animation.ChangeAnimationByEnum(AnimationEnum::IDLE);

        auto& combatable = registry->emplace<CombatableActor>(id);
        combatable.actorType = CombatableActorType::WAVEMOB;
        data->abilityRegistry->RegisterAbility(id, AbilityEnum::ENEMY_AUTOATTACK);
        registry->emplace<HealthBar>(id);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f);
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        // collideable.debugDraw = true;
        collideable.collisionLayer = CollisionLayer::ENEMY;

        data->lightSubSystem->LinkRenderableToLight(id);
        registry->emplace<WavemobState>(id);
        // Always set state last to ensure everything is initialised properly before.
        return id;
    }

    entt::entity GameObjectFactory::createKnight(
        entt::registry* registry, GameData* data, Vector3 position, const char* name)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sgTransform>(id, id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);

        // TODO: There should be a "place actor" function that does below automatically (and calls "occupy grid
        // square").
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z});

        Matrix modelTransform = MatrixScale(0.045f, 0.045f, 0.045f);
        auto& renderable = registry->emplace<Renderable>(
            id, ResourceManager::GetInstance().GetModelDeepCopy(AssetID::MDL_NPC_ARISSA), modelTransform);
        renderable.name = name;

        auto& animation = registry->emplace<Animation>(id, AssetID::MDL_NPC_ARISSA);
        animation.ChangeAnimation(0);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::NPC;
        data->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, id);

        auto& dialogue = registry->emplace<Dialogue>(id);
        dialogue.sentence = "Hello, this is a test sentence.";
        dialogue.conversationPos =
            Vector3Add(transform.GetWorldPos(), Vector3Multiply(transform.forward(), {10.0f, 1, 10.0f}));

        data->lightSubSystem->LinkRenderableToLight(id);
        return id;
    }

    void AttachWeaponToModel(Model& targetModel)
    {
        auto& target = targetModel.meshes[2];
        Model dagger = LoadModel("resources/models/gltf/sword.glb");
        auto* boneIds = target.boneIds;
        auto* boneWeights = target.boneWeights;
        target = dagger.meshes[0];
        target.boneIds = boneIds;
        target.boneWeights = boneWeights;
        targetModel.materials[targetModel.meshMaterial[2]] = dagger.materials[1];
    }

    entt::entity GameObjectFactory::createPlayer(
        entt::registry* registry, GameData* data, Vector3 position, const char* name)
    {
        // TODO: On load, the actor's getFirstCollision box doesn't seem to be correct. Causes a
        // bug that, if I don't move before casting a move, the enemies don't register
        // that the player has a getFirstCollision box.
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sgTransform>(id, id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z});

        Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
        auto& renderable = registry->emplace<Renderable>(
            id, ResourceManager::GetInstance().GetModelDeepCopy(AssetID::MDL_PLAYER_DEFAULT), modelTransform);
        renderable.name = "Player";

        std::vector<Mesh> meshes(
            renderable.GetModel()->GetRlModel().meshes,
            renderable.GetModel()->GetRlModel().meshes + renderable.GetModel()->GetRlModel().meshCount);
        Model& model = renderable.GetModel()->GetRlModel();

        std::vector<int> meshMaterials(model.meshMaterial, model.meshMaterial + model.meshCount);

        // Below swaps sword and dagger meshes
        // int holderIdx = 2;
        // int copyIdx = 3;
        // auto holder = renderable.GetModel()->GetMesh(holderIdx);
        // auto copy = renderable.GetModel()->GetMesh(copyIdx);
        // int matHolder = model.meshMaterial[holderIdx];
        // int matCopy = model.meshMaterial[copyIdx];
        // model.meshes[copyIdx] = holder;
        // model.meshes[holderIdx] = copy;
        // model.meshMaterial[copyIdx] = matHolder;
        // model.meshMaterial[holderIdx] = matCopy;

        auto& targetModel = renderable.GetModel()->GetRlModel();
        // AttachWeaponToModel(targetModel);

        // -= 2 makes player unarmed
        model.meshCount -= 2;

        auto& moveable = registry->emplace<MoveableActor>(id);
        moveable.movementSpeed = 0.35f;
        moveable.pathfindingBounds = 150;

        // Set animation hooks
        auto& animation = registry->emplace<Animation>(id, AssetID::MDL_PLAYER_DEFAULT);

        animation.animationMap[AnimationEnum::WALK] = 1;
        animation.animationMap[AnimationEnum::TALK] = 2;
        animation.animationMap[AnimationEnum::AUTOATTACK] = 6;
        animation.animationMap[AnimationEnum::RUN] = 4;
        animation.animationMap[AnimationEnum::IDLE] = 11; // 10 is normal
        animation.animationMap[AnimationEnum::SPIN] = 5;
        animation.animationMap[AnimationEnum::SLASH] = 6;
        animation.animationMap[AnimationEnum::SPELLCAST_UP] = 7;
        animation.animationMap[AnimationEnum::SPELLCAST_FWD] = 8;
        animation.animationMap[AnimationEnum::ROLL] = 9;
        animation.ChangeAnimationByEnum(AnimationEnum::IDLE);

        {
            entt::sink sink{moveable.onFinishMovement};
            sink.connect<[](Animation& animation, entt::entity entity) {
                animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
            }>(animation);
        }
        {
            entt::sink sink{moveable.onStartMovement};
            sink.connect<[](Animation& animation, entt::entity entity) {
                animation.ChangeAnimationByEnum(AnimationEnum::RUN);
            }>(animation);
        }
        {
            // TODO: Just to test animations on demand
            // entt::sink sink{data->userInput->keyIPressed};
            // sink.connect<[](Animation& animation) {
            //    if (animation.animIndex == 0)
            //    {
            //        animation.ChangeAnimationByEnum(AnimationEnum::TALK);
            //    }
            //    else if (animation.animIndex == 2)
            //    {
            //        animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
            //    }
            //}>(animation);
        }

        auto& controllable = registry->emplace<ControllableActor>(id, id, data->cursor.get());
        data->controllableActorSystem->SetControlledActor(id);

        data->reflectionSignalRouter->CreateHook<entt::entity>(
            id, data->cursor->onFloorClick, controllable.onFloorClick);
        data->reflectionSignalRouter->CreateHook<entt::entity>(
            id, data->cursor->onEnemyLeftClick, controllable.onEnemyLeftClick);
        data->reflectionSignalRouter->CreateHook<entt::entity>(
            id, data->cursor->onEnemyRightClick, controllable.onEnemyRightClick);

        // Combat
        auto& combatable = registry->emplace<CombatableActor>(id);
        combatable.actorType = CombatableActorType::PLAYER;

        // Initialise starting abilities
        data->playerAbilitySystem->SetSlot(0, data->abilityRegistry->RegisterAbility(id, AbilityEnum::WHIRLWIND));
        data->playerAbilitySystem->SetSlot(1, data->abilityRegistry->RegisterAbility(id, AbilityEnum::RAINFOFIRE));
        data->playerAbilitySystem->SetSlot(
            2, data->abilityRegistry->RegisterAbility(id, AbilityEnum::LIGHTNINGBALL));
        data->abilityRegistry->RegisterAbility(id, AbilityEnum::PLAYER_AUTOATTACK);

        // ---

        BoundingBox bb = createRectangularBoundingBox(3.0f, 4.5f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::PLAYER;

        data->lightSubSystem->LinkRenderableToLight(id);

        registry->emplace<PlayerState>(id);
        // Always set state last to ensure everything is initialised properly before.

        auto weaponEntity = registry->create();
        auto& weapon = registry->emplace<Weapon>(weaponEntity);
        weapon.owner = id;
        registry->emplace<Renderable>(
            weaponEntity, LoadModel("resources/models/gltf/dagger.glb"), MatrixScale(0.035, 0.035, 0.035));
        auto& weaponTrans = registry->emplace<sgTransform>(weaponEntity, weaponEntity);
        weaponTrans.SetParent(&transform);
        weaponTrans.SetLocalPos(Vector3Zero());

        return id;
    }

    void GameObjectFactory::createPortal(entt::registry* registry, GameData* data, Vector3 position)
    {
        {

            entt::entity id = registry->create();

            auto& transform = registry->emplace<sgTransform>(id, id);
            GridSquare actorIdx{};
            data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
            float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
            transform.SetPosition({position.x, 12, position.z});
            transform.SetScale(1.0f);
            transform.SetRotation({0, 0, 0});

            auto& timer = registry->emplace<Timer>(id);
            timer.SetMaxTime(1000000);
            timer.Start();

            Texture texture = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_NOISE50);
            Texture texture2 = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_NOISE45);

            Matrix modelTransform = MatrixRotateX(90 * DEG2RAD);

            Model tmp_model = LoadModelFromMesh(GenMeshPlane(20, 20, 1, 1));
            ModelSafe model(tmp_model);
            auto& renderable = registry->emplace<Renderable>(id, std::move(model), modelTransform);
            renderable.name = "Portal";

            Shader shader =
                ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/portal.fs");
            int secondsLoc = GetShaderLocation(shader, "seconds");
            renderable.GetModel()->SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
            renderable.GetModel()->SetTexture(texture2, 0, MATERIAL_MAP_EMISSION);
            shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
            renderable.GetModel()->SetShader(shader, 0);

            renderable.reqShaderUpdate = [data, secondsLoc](entt::entity entity) -> void {
                auto& r = data->registry->get<Renderable>(entity);
                auto& t = data->registry->get<Timer>(entity);
                auto time = t.GetCurrentTime();
                SetShaderValue(r.GetModel()->GetShader(0), secondsLoc, &time, SHADER_UNIFORM_FLOAT);
            };

            BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
            auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
            collideable.collisionLayer = CollisionLayer::BUILDING;
        }

        entt::entity id = registry->create();

        auto& transform = registry->emplace<sgTransform>(id, id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z});
        transform.SetScale(10.0f);
        transform.SetRotation({0, 0, 0});

        Matrix modelTransform = MatrixIdentity();

        auto& renderable = registry->emplace<Renderable>(
            id, ResourceManager::GetInstance().GetModelCopy(AssetID::MDL_BUILDING_PORTAL), modelTransform);
        renderable.name = "Portal Outer";
        data->lightSubSystem->LinkRenderableToLight(id);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::BUILDING;
    }

    void GameObjectFactory::createWizardTower(entt::registry* registry, GameData* data, Vector3 position)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sgTransform>(id, id);
        GridSquare actorIdx{};
        data->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = data->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
        transform.SetPosition({position.x, height, position.z});
        transform.SetScale(1.0f);
        transform.SetRotation({0, 0, 0});

        Matrix modelTransform = MatrixIdentity();
        auto& renderable = registry->emplace<Renderable>(
            id, ResourceManager::GetInstance().GetModelCopy(AssetID::MDL_BUILDING_WIZARDTOWER1), modelTransform);
        renderable.name = "Wizard Tower";
        data->lightSubSystem->LinkRenderableToLight(id);

        BoundingBox bb = renderable.GetModel()->CalcLocalBoundingBox();
        auto& collideable = registry->emplace<Collideable>(id, registry, id, bb);
        collideable.collisionLayer = CollisionLayer::BUILDING;

        registry->emplace<TowerState>(id);
    }

} // namespace sage
