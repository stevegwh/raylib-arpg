//
// Created by Steve Wheeler on 08/11/2024.
//

#include "EquipmentSystem.hpp"

#include "Camera.hpp"
#include "components/Animation.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "components/WeaponComponent.hpp"
#include "ControllableActorSystem.hpp"
#include "GameData.hpp"
#include "LightManager.hpp"
#include "PartySystem.hpp"
#include "ResourceManager.hpp"
#include "slib.hpp"
#include "systems/AnimationSystem.hpp"

#include "components/PartyMemberComponent.hpp"
#include "components/UberShaderComponent.hpp"
#include "raylib.h"
#include "raymath.h"

namespace sage
{

    void EquipmentSystem::updateCharacterWeaponPosition(entt::entity owner) const
    {
        const auto& equipment = registry->get<EquipmentComponent>(owner);
        for (auto [k, entity] : equipment.worldModels)
        {
            if (registry->valid(entity) && registry->any_of<WeaponComponent>(entity))
            {
                auto& weapon = registry->get<WeaponComponent>(entity);
                auto& weaponRend = registry->get<Renderable>(entity);
                auto& model = registry->get<Renderable>(weapon.owner).GetModel()->GetRlModel();
                auto boneId = GetBoneIdByName(model.bones, model.boneCount, weapon.parentBoneName.c_str());
                assert(boneId >= 0);
                auto* matrices = model.meshes[0].boneMatrices;
                auto mat = matrices[boneId];
                mat = MatrixMultiply(weapon.parentSocket, mat);
                mat = MatrixMultiply(mat, weaponRend.initialTransform);
                weaponRend.GetModel()->SetTransform(mat);
            }
        }
    }

    void EquipmentSystem::updateCharacterPreviewPose(entt::entity entity)
    {
        auto& animation = registry->get<Animation>(entity);
        auto& renderable = registry->get<Renderable>(entity);
        auto& animData = animation.current;
        const ModelAnimation& anim = animation.animations[animData.index];
        animData.currentFrame = anim.frameCount;
        renderable.GetModel()->UpdateAnimation(anim, animData.currentFrame);
        animation.onAnimationUpdated.Publish(entity);
    }

    void EquipmentSystem::instantiateWeapon(
        entt::entity owner, entt::entity itemId, EquipmentSlotName itemType) const
    {
        auto weaponEntity = registry->create();
        auto& equipment = registry->get<EquipmentComponent>(owner);

        if (equipment.worldModels.contains(itemType) && registry->valid(equipment.worldModels[itemType]))
        {
            registry->destroy(equipment.worldModels[itemType]);
        }
        equipment.worldModels[itemType] = weaponEntity;

        Matrix weaponMat;
        {
            // Hard coded location of the "socket" for the weapon
            // TODO: Export sockets as txt and store their transform in weaponSocket
            auto translation = Vector3{-86.803f, 159.62f, 6.0585f};
            Quaternion rotation{0.021f, -0.090f, 0.059f, 0.994f};
            auto scale = Vector3{1, 1, 1};
            weaponMat = ComposeMatrix(translation, rotation, scale);
        }

        auto& weapon = registry->emplace<WeaponComponent>(weaponEntity);
        weapon.parentSocket = weaponMat;
        weapon.parentBoneName = "mixamorig:RightHand";
        weapon.owner = owner;
        auto& renderable = registry->get<Renderable>(owner);
        auto& transform = registry->get<sgTransform>(owner);
        auto& item = registry->get<ItemComponent>(itemId);
        registry->emplace<Renderable>(
            weaponEntity, ResourceManager::GetInstance().GetModelCopy(item.model), renderable.initialTransform);
        auto& uber =
            registry->emplace<UberShaderComponent>(weaponEntity, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(UberShaderComponent::Flags::Lit);

        auto& weaponTrans = registry->emplace<sgTransform>(weaponEntity, weaponEntity);
        weaponTrans.SetParent(&transform);
        weaponTrans.SetLocalPos(Vector3Zero());
        weaponTrans.SetLocalRot({0, 0, 0, 0});

        auto& animation = registry->get<Animation>(owner);
        animation.onAnimationUpdated.Subscribe(
            [this](entt::entity _entity) { updateCharacterWeaponPosition(_entity); });
    }

    void EquipmentSystem::GeneratePortraitRenderTexture(entt::entity entity, float width, float height)
    {
        auto& info = registry->get<PartyMemberComponent>(entity);
        // TODO: inefficient
        UnloadTexture(info.portraitImg.texture);
        info.portraitImg = LoadRenderTexture(width, height);

        auto& transform = registry->get<sgTransform>(entity);
        auto& renderable = registry->get<Renderable>(entity);
        auto& animation = registry->get<Animation>(entity);

        // TODO: Should probably update the weapons again after taking the "photo"

        auto oldPos = transform.GetWorldPos();
        auto cameraPos = gameData->camera->GetPosition();
        auto cameraTarget = gameData->camera->getRaylibCam()->target;

        transform.SetPosition({0, -999, 0});

        auto current = animation.current;
        animation.ChangeAnimationByEnum(AnimationEnum::IDLE2);
        updateCharacterPreviewPose(entity);
        gameData->camera->SetCamera({-1.5, -992.5, 1.5}, {0.5, -992.5, 0});

        info.portraitImg = LoadRenderTexture(width, height);
        BeginTextureMode(info.portraitImg);
        ClearBackground(BLACK);
        BeginMode3D(*gameData->camera->getRaylibCam());
        auto& uber = registry->get<UberShaderComponent>(entity);
        uber.ClearFlagAll(UberShaderComponent::Lit);
        uber.SetShaderBools();
        uber.SetFlagAll(UberShaderComponent::Lit);
        renderable.GetModel()->Draw(transform.GetWorldPos(), transform.GetScale().x, WHITE);

        EndMode3D();
        EndTextureMode();

        animation.current = current;
        transform.SetPosition(oldPos);
        gameData->camera->SetCamera(cameraPos, cameraTarget);
    }

    void EquipmentSystem::GenerateRenderTexture(entt::entity entity, float width, float height)
    {
        auto& equipment = registry->get<EquipmentComponent>(entity);
        auto& transform = registry->get<sgTransform>(entity);
        auto& renderable = registry->get<Renderable>(entity);
        auto& animation = registry->get<Animation>(entity);

        // TODO: Should probably update the weapons again after taking the "photo"

        auto oldPos = transform.GetWorldPos();
        auto cameraPos = gameData->camera->GetPosition();
        auto cameraTarget = gameData->camera->getRaylibCam()->target;

        transform.SetPosition({0, -999, 0});
        gameData->camera->SetCamera({6, -996, 12}, {0, -996, 0});

        auto current = animation.current;
        animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
        updateCharacterPreviewPose(entity);

        // TODO: inefficient
        UnloadTexture(equipment.renderTexture.texture);
        equipment.renderTexture = LoadRenderTexture(width, height);

        BeginTextureMode(equipment.renderTexture);
        ClearBackground(BLANK);
        BeginMode3D(*gameData->camera->getRaylibCam());
        auto& uber = registry->get<UberShaderComponent>(entity);
        uber.ClearFlagAll(UberShaderComponent::Lit);
        uber.SetShaderBools();
        uber.SetFlagAll(UberShaderComponent::Lit);
        renderable.GetModel()->Draw(transform.GetWorldPos(), transform.GetScale().x, WHITE);

        if (equipment.worldModels.contains(EquipmentSlotName::LEFTHAND))
        {
            if (registry->valid(equipment.worldModels[EquipmentSlotName::LEFTHAND]))
            {
                auto& leftHandRenderable =
                    registry->get<Renderable>(equipment.worldModels[EquipmentSlotName::LEFTHAND]);
                auto& leftHandTrans =
                    registry->get<sgTransform>(equipment.worldModels[EquipmentSlotName::LEFTHAND]);
                auto& weaponUber =
                    registry->get<UberShaderComponent>(equipment.worldModels[EquipmentSlotName::LEFTHAND]);
                weaponUber.ClearFlagAll(UberShaderComponent::Lit);
                weaponUber.SetShaderBools();
                weaponUber.SetFlagAll(UberShaderComponent::Lit);
                leftHandRenderable.GetModel()->Draw(
                    leftHandTrans.GetWorldPos(), leftHandTrans.GetScale().x, WHITE);
            }
        }

        EndMode3D();
        EndTextureMode();

        animation.current = current;
        transform.SetPosition(oldPos);
        gameData->camera->SetCamera(cameraPos, cameraTarget);
    }

    entt::entity EquipmentSystem::GetItem(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (!equipment.slots.contains(itemType)) return entt::null;
        return equipment.slots[itemType];
    }

    void EquipmentSystem::EquipItem(entt::entity owner, entt::entity item, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        equipment.slots[itemType] = item;
        if (itemType == EquipmentSlotName::LEFTHAND)
        {
            instantiateWeapon(owner, item, itemType);
        }
        onEquipmentUpdated.Publish(owner);
    }

    void EquipmentSystem::MoveItemToInventory(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (equipment.slots[itemType] != entt::null)
        {
            auto& inventory = registry->get<InventoryComponent>(owner);
            if (!inventory.AddItem(equipment.slots[itemType]))
            {
                // TODO: handle inventory full.
                return;
            }
            registry->destroy(equipment.worldModels[itemType]);
            equipment.worldModels[itemType] = entt::null;
        }
        equipment.slots[itemType] = entt::null;
        onEquipmentUpdated.Publish(owner);
    }

    void EquipmentSystem::DestroyItem(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (equipment.slots[itemType] != entt::null)
        {
            if (equipment.worldModels.contains(itemType) && equipment.worldModels[itemType] != entt::null)
            {
                registry->destroy(equipment.worldModels[itemType]);
                equipment.worldModels[itemType] = entt::null;
            }
            equipment.slots[itemType] = entt::null;
            onEquipmentUpdated.Publish(owner);
        }
    }

    bool EquipmentSystem::SwapItems(entt::entity owner, EquipmentSlotName itemType1, EquipmentSlotName itemType2)
    {
        bool bothRings = (itemType1 == EquipmentSlotName::RING1 || itemType1 == EquipmentSlotName::RING2) &&
                         (itemType2 == EquipmentSlotName::RING1 || itemType2 == EquipmentSlotName::RING2);

        bool bothHands = (itemType1 == EquipmentSlotName::LEFTHAND || itemType1 == EquipmentSlotName::RIGHTHAND) &&
                         (itemType2 == EquipmentSlotName::LEFTHAND || itemType2 == EquipmentSlotName::RIGHTHAND);

        if (!bothRings && !bothHands)
        {
            return false;
        }

        auto& equipment = registry->get<EquipmentComponent>(owner);

        entt::entity item1 = entt::null;
        entt::entity item2 = entt::null;

        if (equipment.slots.contains(itemType1) && equipment.slots[itemType1] != entt::null)
        {
            item1 = equipment.slots[itemType1];
        }

        if (equipment.slots.contains(itemType2) && equipment.slots[itemType2] != entt::null)
        {
            item2 = equipment.slots[itemType2];
        }

        if (item1 != entt::null)
        {
            DestroyItem(owner, itemType1);
        }
        if (item2 != entt::null)
        {
            DestroyItem(owner, itemType2);
        }

        if (item1 != entt::null)
        {
            EquipItem(owner, item1, itemType2);
        }
        if (item2 != entt::null)
        {
            EquipItem(owner, item2, itemType1);
        }

        return true;
    }

    void EquipmentSystem::onComponentAdded(entt::entity addedEntity)
    {
    }

    void EquipmentSystem::onComponentRemoved(entt::entity removedEntity)
    {
    }

    EquipmentSystem::EquipmentSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        registry->on_construct<EquipmentComponent>().connect<&EquipmentSystem::onComponentAdded>(this);
        registry->on_destroy<EquipmentComponent>().connect<&EquipmentSystem::onComponentRemoved>(this);
    }
} // namespace sage