//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"

#include "AbilityFactory.hpp"
#include "components/CombatableActor.hpp"
#include "components/DialogComponent.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/HealthBar.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/States.hpp"
#include "ItemFactory.hpp"
#include "Systems.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "components/ControllableActor.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/MoveableActor.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/LightManager.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/slib.hpp"
#include "engine/systems/NavigationGridSystem.hpp"
#include "engine/Timer.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "engine/Cursor.hpp"
#include "raymath.h"

namespace lq
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

    void placeActor(entt::registry* registry, entt::entity entity, Systems* sys, Vector3 position)
    {
        auto& transform = registry->get<sage::sgTransform>(entity);
        sage::GridSquare actorIdx{};
        sys->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        auto gs = sys->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col);
        assert(gs);
        float height = gs->GetTerrainHeight();
        transform.SetPosition({position.x, height, position.z});
    }

    entt::entity GameObjectFactory::createDialogCutscene(
        entt::registry* registry, Vector3 position, const char* name)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id, id);
        transform.SetPosition(position);

        auto& renderable = registry->emplace<sage::Renderable>(id);
        renderable.SetName(name);
        renderable.Disable();
        registry->emplace<DialogComponent>(id);

        return id;
    }

    entt::entity GameObjectFactory::createEnemy(
        entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation, const char* name)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id, id);
        placeActor(registry, id, sys, position);

        auto& moveable = registry->emplace<sage::MoveableActor>(id);
        moveable.movementSpeed = 0.25f;

        Matrix modelTransform = MatrixScale(0.03f, 0.03f, 0.03f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelDeepCopy("mdl_goblin"), modelTransform);
        renderable.SetName(name);
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        auto& animation = registry->emplace<sage::Animation>(id, "mdl_goblin");
        animation.animationMap[sage::AnimationEnum::IDLE] = 1;
        animation.animationMap[sage::AnimationEnum::DEATH] = 0;
        animation.animationMap[sage::AnimationEnum::WALK] = 4;
        animation.animationMap[sage::AnimationEnum::AUTOATTACK] = 2;
        animation.ChangeAnimationByEnum(sage::AnimationEnum::IDLE);

        auto& combatable = registry->emplace<CombatableActor>(id);
        combatable.actorType = CombatableActorType::WAVEMOB;
        sys->abilityFactory->RegisterAbility(id, AbilityEnum::ENEMY_AUTOATTACK);
        registry->emplace<HealthBar>(id);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f);
        auto& collideable = registry->emplace<sage::Collideable>(id, registry, id, bb);
        collideable.collisionLayer = sage::CollisionLayer::ENEMY;

        transform.SetRotation(
            rotation); // TODO: Find out why this must be called after bounding box from collideable is created

        registry->emplace<WavemobState>(id);
        return id;
    }

    entt::entity GameObjectFactory::createGoblinNPC(
        entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation, const char* name)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id, id);
        placeActor(registry, id, sys, position);

        Matrix modelTransform = MatrixScale(0.03f, 0.03f, 0.03f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelDeepCopy("mdl_goblin"), modelTransform);
        renderable.SetName(name);
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        auto& animation = registry->emplace<sage::Animation>(id, "mdl_goblin");
        animation.animationMap[sage::AnimationEnum::IDLE] = 1;
        animation.animationMap[sage::AnimationEnum::DEATH] = 0;
        animation.animationMap[sage::AnimationEnum::WALK] = 4;
        animation.animationMap[sage::AnimationEnum::AUTOATTACK] = 2;
        animation.animationMap[sage::AnimationEnum::TALK] = 1;
        animation.ChangeAnimationByEnum(sage::AnimationEnum::IDLE);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<sage::Collideable>(id, registry, id, bb);
        collideable.collisionLayer = sage::CollisionLayer::NPC;
        transform.SetRotation(rotation);
        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, id);

        registry->emplace<DialogComponent>(id);

        return id;
    }

    entt::entity GameObjectFactory::createArissa(
        entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id, id);
        placeActor(registry, id, sys, position);

        Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelDeepCopy("mdl_player_default"), modelTransform);
        renderable.SetName("Arissa");
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        // Set animation hooks
        auto& animation = registry->emplace<sage::Animation>(id, "mdl_player_default");
        // TODO: I think we're going to need to move these elsewhere to make this function more generic
        animation.animationMap[sage::AnimationEnum::WALK] = 1;
        animation.animationMap[sage::AnimationEnum::TALK] = 2;
        animation.animationMap[sage::AnimationEnum::AUTOATTACK] = 6;
        animation.animationMap[sage::AnimationEnum::RUN] = 4;
        animation.animationMap[sage::AnimationEnum::IDLE2] = 0;
        animation.animationMap[sage::AnimationEnum::IDLE] = 10; // 11 is T-Pose, 10 is ninja idle
        animation.animationMap[sage::AnimationEnum::SPIN] = 5;
        animation.animationMap[sage::AnimationEnum::SLASH] = 6;
        animation.animationMap[sage::AnimationEnum::SPELLCAST_UP] = 7;
        animation.animationMap[sage::AnimationEnum::SPELLCAST_FWD] = 8;
        animation.animationMap[sage::AnimationEnum::ROLL] = 9;
        animation.ChangeAnimationByEnum(sage::AnimationEnum::IDLE);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<sage::Collideable>(id, registry, id, bb);
        collideable.collisionLayer = sage::CollisionLayer::NPC;
        transform.SetRotation(rotation);
        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, id);

        registry->emplace<DialogComponent>(id);

        return id;
    }

    entt::entity GameObjectFactory::createPlayer(
        entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation, const char* name)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id, id);
        placeActor(registry, id, sys, position);

        Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelDeepCopy("mdl_player_default"), modelTransform);
        renderable.SetName(name);
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        auto& moveable = registry->emplace<sage::MoveableActor>(id);
        moveable.movementSpeed = 0.35f;
        moveable.pathfindingBounds = 100;

        BoundingBox bb = createRectangularBoundingBox(3.0f, 6.5f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<sage::Collideable>(id, registry, id, bb);
        collideable.collisionLayer = sage::CollisionLayer::PLAYER;
        transform.SetRotation(rotation);

        // Set animation hooks
        auto& animation = registry->emplace<sage::Animation>(id, "mdl_player_default");
        // TODO: I think we're going to need to move these elsewhere to make this function more generic
        animation.animationMap[sage::AnimationEnum::WALK] = 1;
        animation.animationMap[sage::AnimationEnum::TALK] = 2;
        animation.animationMap[sage::AnimationEnum::AUTOATTACK] = 6;
        animation.animationMap[sage::AnimationEnum::RUN] = 4;
        animation.animationMap[sage::AnimationEnum::IDLE2] = 0;
        animation.animationMap[sage::AnimationEnum::IDLE] = 10; // 11 is T-Pose, 10 is ninja idle
        animation.animationMap[sage::AnimationEnum::SPIN] = 5;
        animation.animationMap[sage::AnimationEnum::SLASH] = 6;
        animation.animationMap[sage::AnimationEnum::SPELLCAST_UP] = 7;
        animation.animationMap[sage::AnimationEnum::SPELLCAST_FWD] = 8;
        animation.animationMap[sage::AnimationEnum::ROLL] = 9;
        animation.ChangeAnimationByEnum(sage::AnimationEnum::IDLE);

        registry->emplace<PartyMemberComponent>(id, id);
        sys->partySystem->AddMember(id);
        registry->emplace<ControllableActor>(id);
        sys->cursor->SetSelectedActor(id);
        registry->emplace<DialogComponent>(id);

        // Combat
        auto& combatable = registry->emplace<CombatableActor>(id);
        combatable.actorType = CombatableActorType::PLAYER;

        // TODO: Move elsewhere/read from save file
        // Initialise starting abilities
        sys->playerAbilitySystem->SetSlot(0, sys->abilityFactory->RegisterAbility(id, AbilityEnum::WHIRLWIND));
        sys->playerAbilitySystem->SetSlot(1, sys->abilityFactory->RegisterAbility(id, AbilityEnum::RAINFOFIRE));
        sys->abilityFactory->RegisterAbility(id, AbilityEnum::PLAYER_AUTOATTACK);

        registry->emplace<InventoryComponent>(id);
        registry->emplace<EquipmentComponent>(id);

        return id;
    }

    void GameObjectFactory::createPortal(entt::registry* registry, Systems* sys, Vector3 position)
    {
        {
            entt::entity id = registry->create();

            auto& transform = registry->emplace<sage::sgTransform>(id, id);
            sage::GridSquare actorIdx{};
            sys->navigationGridSystem->WorldToGridSpace(position, actorIdx);
            float height =
                sys->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->GetTerrainHeight();
            transform.SetPosition({position.x, 12, position.z});
            transform.SetScale(1.0f);
            transform.SetRotation({0, 0, 0});

            auto& timer = registry->emplace<Timer>(id);
            timer.SetMaxTime(1000000);
            timer.Start();

            Texture texture = sage::ResourceManager::GetInstance().TextureLoad("T_Random_50");
            Texture texture2 = sage::ResourceManager::GetInstance().TextureLoad("T_Random_45");

            Matrix modelTransform = MatrixRotateX(90 * DEG2RAD);

            Model tmp_model = LoadModelFromMesh(GenMeshPlane(20, 20, 1, 1));
            sage::ModelSafe model(tmp_model);
            auto& renderable = registry->emplace<sage::Renderable>(id, std::move(model), modelTransform);
            renderable.SetName("Portal");

            Shader shader =
                sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/portal.fs");
            int secondsLoc = GetShaderLocation(shader, "seconds");
            renderable.GetModel()->SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
            renderable.GetModel()->SetTexture(texture2, 0, MATERIAL_MAP_EMISSION);
            shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
            renderable.GetModel()->SetShader(shader, 0);

            renderable.reqShaderUpdate = [sys, secondsLoc](entt::entity entity) -> void {
                auto& r = sys->registry->get<sage::Renderable>(entity);
                auto& t = sys->registry->get<Timer>(entity);
                auto time = t.GetCurrentTime();
                SetShaderValue(r.GetModel()->GetShader(0), secondsLoc, &time, SHADER_UNIFORM_FLOAT);
            };

            BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
            auto& collideable = registry->emplace<sage::Collideable>(id, registry, id, bb);
            collideable.collisionLayer = sage::CollisionLayer::BUILDING;
        }

        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id, id);
        sage::GridSquare actorIdx{};
        sys->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = sys->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->GetTerrainHeight();
        transform.SetPosition({position.x, height, position.z});
        transform.SetScale(10.0f);
        transform.SetRotation({0, 0, 0});

        Matrix modelTransform = MatrixIdentity();

        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelCopy("MDL_BUILDING_PORTAL"), modelTransform);
        renderable.SetName("Portal Outer");
        sys->lightSubSystem->LinkRenderableToLight(id);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable = registry->emplace<sage::Collideable>(id, registry, id, bb);
        collideable.collisionLayer = sage::CollisionLayer::BUILDING;
    }

    void GameObjectFactory::createWizardTower(entt::registry* registry, Systems* sys, Vector3 position)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id, id);
        sage::GridSquare actorIdx{};
        sys->navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height = sys->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->GetTerrainHeight();
        transform.SetPosition({position.x, height, position.z});
        transform.SetScale(1.0f);
        transform.SetRotation({0, 0, 0});

        Matrix modelTransform = MatrixIdentity();
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelCopy("MDL_BUILDING_WIZARDTOWER1"), modelTransform);
        renderable.SetName("Wizard Tower");
        sys->lightSubSystem->LinkRenderableToLight(id);

        BoundingBox bb = renderable.GetModel()->CalcLocalBoundingBox();
        auto& collideable = registry->emplace<sage::Collideable>(id, registry, id, bb);
        collideable.collisionLayer = sage::CollisionLayer::BUILDING;
    }

    bool GameObjectFactory::spawnItemInWorld(entt::registry* registry, entt::entity itemId, Vector3 position)
    {
        auto& item = registry->get<ItemComponent>(itemId);
        if (item.HasFlag(ItemFlags::QUEST)) return false;
        auto model = sage::ResourceManager::GetInstance().GetModelCopy(item.model);
        // TODO: Need a way to store the matrix scale? Maybe in the resource packer we should store the transform
        auto& renderable =
            registry->emplace<sage::Renderable>(itemId, std::move(model), MatrixScale(0.035, 0.035, 0.035));
        auto& transform = registry->emplace<sage::sgTransform>(itemId, itemId);
        transform.SetPosition(position);

        // TODO: Uber shader? Lit?

        auto& collideable = registry->emplace<sage::Collideable>(
            itemId, createRectangularBoundingBox(2.0, 2.0), transform.GetMatrixNoRot());
        collideable.collisionLayer = sage::CollisionLayer::ITEM;
        return true;
    }

} // namespace lq
