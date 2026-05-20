//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "AbilityFactory.hpp"
#include "collision/RpgCollisionLayers.hpp"
#include "components/CombatableActor.hpp"
#include "components/DialogComponent.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/HealthBar.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "ItemFactory.hpp"
#include "Systems.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "systems/states/WavemobStates.hpp"

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
#include "engine/systems/TransformSystem.hpp"
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
        sage::GridSquare actorIdx{};
        sys->engine.navigationGridSystem->WorldToGridSpace(position, actorIdx);
        auto gs = sys->engine.navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col);
        assert(gs);
        float height = gs->heightMap.GetHeight();
        sys->engine.transformSystem->SetPosition(entity, {position.x, height, position.z});
    }

    entt::entity GameObjectFactory::createDialogCutscene(
        entt::registry* registry, Systems* sys, Vector3 position, const char* name)
    {
        entt::entity id = registry->create();
        registry->emplace<sage::sgTransform>(id);
        sys->engine.transformSystem->SetPosition(id, position);

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

        auto& transform = registry->emplace<sage::sgTransform>(id);
        placeActor(registry, id, sys, position);

        auto& moveable = registry->emplace<sage::MoveableActor>(id);
        moveable.movementSpeed = 0.25f;

        Matrix modelTransform = MatrixScale(0.03f, 0.03f, 0.03f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().CreateModelMutable("mdl_goblin"), modelTransform);
        renderable.SetName(name);
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        auto& animation = registry->emplace<sage::Animation>(id, "mdl_goblin");
        animation.animationMap[lq::animation_ids::Idle] = 1;
        animation.animationMap[lq::animation_ids::Death] = 0;
        animation.animationMap[lq::animation_ids::Walk] = 4;
        animation.animationMap[lq::animation_ids::AutoAttack] = 2;
        animation.ChangeAnimationById(lq::animation_ids::Idle);

        auto& combatable = registry->emplace<CombatableActor>(id);
        combatable.actorType = CombatableActorType::WAVEMOB;
        sys->abilityFactory->RegisterAbility(id, AbilityEnum::ENEMY_AUTOATTACK);
        registry->emplace<HealthBar>(id);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f);
        auto& collideable =
            registry->emplace<sage::Collideable>(id, bb, registry->get<sage::sgTransform>(id).GetMatrixNoRot());
        collideable.SetCollisionLayer(
            lq::collision_layers::Enemy, lq::collision_masks::ForLayer(lq::collision_layers::Enemy));
        sys->engine.transformSystem->SetRotation(
            id, rotation); // TODO: Find out why this must be called after bounding box from collideable is created

        registry->emplace<WavemobState>(id);
        return id;
    }

    entt::entity GameObjectFactory::createGoblinNPC(
        entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation, const char* name)
    {
        entt::entity id = registry->create();

        registry->emplace<sage::sgTransform>(id);
        placeActor(registry, id, sys, position);

        Matrix modelTransform = MatrixScale(0.03f, 0.03f, 0.03f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().CreateModelMutable("mdl_goblin"), modelTransform);
        renderable.SetName(name);
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        auto& animation = registry->emplace<sage::Animation>(id, "mdl_goblin");
        animation.animationMap[lq::animation_ids::Idle] = 1;
        animation.animationMap[lq::animation_ids::Death] = 0;
        animation.animationMap[lq::animation_ids::Walk] = 4;
        animation.animationMap[lq::animation_ids::AutoAttack] = 2;
        animation.animationMap[lq::animation_ids::Talk] = 1;
        animation.ChangeAnimationById(lq::animation_ids::Idle);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable =
            registry->emplace<sage::Collideable>(id, bb, registry->get<sage::sgTransform>(id).GetMatrixNoRot());
        collideable.SetCollisionLayer(
            lq::collision_layers::Npc, lq::collision_masks::ForLayer(lq::collision_layers::Npc));
        sys->engine.transformSystem->SetRotation(id, rotation);
        sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, id);

        registry->emplace<DialogComponent>(id);

        return id;
    }

    entt::entity GameObjectFactory::createArissa(
        entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation)
    {
        entt::entity id = registry->create();

        registry->emplace<sage::sgTransform>(id);
        placeActor(registry, id, sys, position);

        Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().CreateModelMutable("mdl_player_default"), modelTransform);
        renderable.SetName("Arissa");
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        // Set animation hooks
        auto& animation = registry->emplace<sage::Animation>(id, "mdl_player_default");
        // TODO: I think we're going to need to move these elsewhere to make this function more generic
        animation.animationMap[lq::animation_ids::Walk] = 1;
        animation.animationMap[lq::animation_ids::Talk] = 2;
        animation.animationMap[lq::animation_ids::AutoAttack] = 6;
        animation.animationMap[lq::animation_ids::Run] = 4;
        animation.animationMap[lq::animation_ids::Idle2] = 0;
        animation.animationMap[lq::animation_ids::Idle] = 10; // 11 is T-Pose, 10 is ninja idle
        animation.animationMap[lq::animation_ids::Spin] = 5;
        animation.animationMap[lq::animation_ids::Slash] = 6;
        animation.animationMap[lq::animation_ids::SpellcastUp] = 7;
        animation.animationMap[lq::animation_ids::SpellcastForward] = 8;
        animation.animationMap[lq::animation_ids::Roll] = 9;
        animation.ChangeAnimationById(lq::animation_ids::Idle);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable =
            registry->emplace<sage::Collideable>(id, bb, registry->get<sage::sgTransform>(id).GetMatrixNoRot());
        collideable.SetCollisionLayer(
            lq::collision_layers::Npc, lq::collision_masks::ForLayer(lq::collision_layers::Npc));
        sys->engine.transformSystem->SetRotation(id, rotation);
        sys->engine.navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, id);

        registry->emplace<DialogComponent>(id);

        return id;
    }

    entt::entity GameObjectFactory::createPlayer(
        entt::registry* registry, Systems* sys, Vector3 position, Vector3 rotation, const char* name)
    {
        entt::entity id = registry->create();

        registry->emplace<sage::sgTransform>(id);
        placeActor(registry, id, sys, position);

        Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().CreateModelMutable("mdl_player_default"), modelTransform);
        renderable.SetName(name);
        auto& uber = registry->emplace<sage::UberShaderComponent>(id, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Skinned);

        auto& moveable = registry->emplace<sage::MoveableActor>(id);
        moveable.movementSpeed = 0.35f;
        moveable.pathfindingBounds = 100;

        BoundingBox bb = createRectangularBoundingBox(3.0f, 6.5f); // Manually set bounding box dimensions
        auto& collideable =
            registry->emplace<sage::Collideable>(id, bb, registry->get<sage::sgTransform>(id).GetMatrixNoRot());
        collideable.SetCollisionLayer(
            lq::collision_layers::Player, lq::collision_masks::ForLayer(lq::collision_layers::Player));
        sys->engine.transformSystem->SetRotation(id, rotation);

        // Set animation hooks
        auto& animation = registry->emplace<sage::Animation>(id, "mdl_player_default");
        // TODO: I think we're going to need to move these elsewhere to make this function more generic
        animation.animationMap[lq::animation_ids::Walk] = 1;
        animation.animationMap[lq::animation_ids::Talk] = 2;
        animation.animationMap[lq::animation_ids::AutoAttack] = 6;
        animation.animationMap[lq::animation_ids::Run] = 4;
        animation.animationMap[lq::animation_ids::Idle2] = 0;
        animation.animationMap[lq::animation_ids::Idle] = 10; // 11 is T-Pose, 10 is ninja idle
        animation.animationMap[lq::animation_ids::Spin] = 5;
        animation.animationMap[lq::animation_ids::Slash] = 6;
        animation.animationMap[lq::animation_ids::SpellcastUp] = 7;
        animation.animationMap[lq::animation_ids::SpellcastForward] = 8;
        animation.animationMap[lq::animation_ids::Roll] = 9;
        animation.ChangeAnimationById(lq::animation_ids::Idle);

        registry->emplace<PartyMemberComponent>(id, id);
        sys->partySystem->AddMember(id);
        registry->emplace<ControllableActor>(id);
        sys->selectionSystem->SetSelectedActor(id);
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

            registry->emplace<sage::sgTransform>(id);
            sage::GridSquare actorIdx{};
            sys->engine.navigationGridSystem->WorldToGridSpace(position, actorIdx);
            float height =
                sys->engine.navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->heightMap.GetHeight();
            sys->engine.transformSystem->SetPosition(id, {position.x, 12, position.z});
            sys->engine.transformSystem->SetScale(id, 1.0f);
            sys->engine.transformSystem->SetRotation(id, {0, 0, 0});

            auto& timer = registry->emplace<Timer>(id);
            timer.SetMaxTime(1000000);
            timer.Start();

            Texture texture = sage::ResourceManager::GetInstance().TextureLoad("T_Random_50");
            Texture texture2 = sage::ResourceManager::GetInstance().TextureLoad("T_Random_45");

            // Portal plane: scale the canonical unit primitive_plane by 20 via the model transform.
            Matrix modelTransform = MatrixMultiply(MatrixScale(20.0f, 20.0f, 1.0f), MatrixRotateX(90 * DEG2RAD));

            auto portalModel = sage::ResourceManager::GetInstance().CreateModelMutable("primitive_plane");

            Shader shader =
                sage::ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/portal.fs");
            int secondsLoc = GetShaderLocation(shader, "seconds");
            portalModel.SetTexture(texture, 0, MATERIAL_MAP_DIFFUSE);
            portalModel.SetTexture(texture2, 0, MATERIAL_MAP_EMISSION);
            shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "texture1");
            portalModel.SetShader(shader, 0);

            auto& renderable = registry->emplace<sage::Renderable>(id, std::move(portalModel), modelTransform);
            renderable.SetName("Portal");

            renderable.reqShaderUpdate = [sys, secondsLoc](entt::entity entity) -> void {
                auto& r = sys->engine.registry->get<sage::Renderable>(entity);
                auto& t = sys->engine.registry->get<Timer>(entity);
                auto time = t.GetCurrentTime();
                SetShaderValue(r.GetModel()->GetShader(0), secondsLoc, &time, SHADER_UNIFORM_FLOAT);
            };

            BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
            auto& collideable = registry->emplace<sage::Collideable>(
                id, bb, registry->get<sage::sgTransform>(id).GetMatrixNoRot());
            collideable.SetCollisionLayer(
                lq::collision_layers::Building, lq::collision_masks::ForLayer(lq::collision_layers::Building));
            collideable.blocksNavigation = true;
            collideable.isStatic = true;
        }

        entt::entity id = registry->create();

        registry->emplace<sage::sgTransform>(id);
        sage::GridSquare actorIdx{};
        sys->engine.navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height =
            sys->engine.navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->heightMap.GetHeight();
        sys->engine.transformSystem->SetPosition(id, {position.x, height, position.z});
        sys->engine.transformSystem->SetScale(id, 10.0f);
        sys->engine.transformSystem->SetRotation(id, {0, 0, 0});

        Matrix modelTransform = MatrixIdentity();

        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelView("MDL_BUILDING_PORTAL"), modelTransform);
        renderable.SetName("Portal Outer");
        sys->engine.lightSubSystem->LinkRenderableToLight(id);

        BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
        auto& collideable =
            registry->emplace<sage::Collideable>(id, bb, registry->get<sage::sgTransform>(id).GetMatrixNoRot());
        collideable.SetCollisionLayer(
            lq::collision_layers::Building, lq::collision_masks::ForLayer(lq::collision_layers::Building));
        collideable.blocksNavigation = true;
        collideable.isStatic = true;
    }

    void GameObjectFactory::createWizardTower(entt::registry* registry, Systems* sys, Vector3 position)
    {
        entt::entity id = registry->create();

        auto& transform = registry->emplace<sage::sgTransform>(id);
        sage::GridSquare actorIdx{};
        sys->engine.navigationGridSystem->WorldToGridSpace(position, actorIdx);
        float height =
            sys->engine.navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->heightMap.GetHeight();
        sys->engine.transformSystem->SetPosition(id, {position.x, height, position.z});
        sys->engine.transformSystem->SetScale(id, 1.0f);
        sys->engine.transformSystem->SetRotation(id, {0, 0, 0});

        Matrix modelTransform = MatrixIdentity();
        auto& renderable = registry->emplace<sage::Renderable>(
            id, sage::ResourceManager::GetInstance().GetModelView("MDL_BUILDING_WIZARDTOWER1"), modelTransform);
        renderable.SetName("Wizard Tower");
        sys->engine.lightSubSystem->LinkRenderableToLight(id);

        BoundingBox bb = renderable.GetModel()->CalcLocalBoundingBox();
        auto& collideable =
            registry->emplace<sage::Collideable>(id, bb, registry->get<sage::sgTransform>(id).GetMatrixNoRot());
        collideable.SetCollisionLayer(
            lq::collision_layers::Building, lq::collision_masks::ForLayer(lq::collision_layers::Building));
        collideable.blocksNavigation = true;
        collideable.isStatic = true;
    }

    bool GameObjectFactory::spawnItemInWorld(
        entt::registry* registry, Systems* sys, entt::entity itemId, Vector3 position)
    {
        auto& item = registry->get<ItemComponent>(itemId);
        if (item.HasFlag(ItemFlags::QUEST)) return false;
        auto model = sage::ResourceManager::GetInstance().GetModelView(item.model);
        // TODO: Need a way to store the matrix scale? Maybe in the resource packer we should store the transform
        registry->emplace<sage::Renderable>(itemId, std::move(model), MatrixScale(0.035, 0.035, 0.035));
        const auto& transform = registry->emplace<sage::sgTransform>(itemId);
        sys->engine.transformSystem->SetPosition(itemId, position);

        // TODO: Uber shader? Lit?

        auto& collideable = registry->emplace<sage::Collideable>(
            itemId, createRectangularBoundingBox(2.0, 2.0), transform.GetMatrixNoRot());
        collideable.SetCollisionLayer(
            lq::collision_layers::Item, lq::collision_masks::ForLayer(lq::collision_layers::Item));
        collideable.isStatic = true;
        return true;
    }

} // namespace lq
