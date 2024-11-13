//
// Created by Steve Wheeler on 08/11/2024.
//

#include "EquipmentSystem.hpp"

#include "Camera.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/Renderable.hpp"
#include "components/WeaponComponent.hpp"
#include "ControllableActorSystem.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "slib.hpp"
#include "systems/LightSubSystem.hpp"

#include "components/Animation.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/sgTransform.hpp"
#include "raylib.h"
#include "raymath.h"

namespace sage
{

    void EquipmentSystem::initRenderTextureScene()
    {
        if (renderTextureSceneInitialised) return;
        renderTextureSceneInitialised = true;
        auto shader = ResourceManager::GetInstance().ShaderLoad(
            "resources/shaders/custom/litskinning.vs", "resources/shaders/custom/litskinning.fs");
        gameData->lightSubSystem->CreateLight(shader, LightType::LIGHT_POINT, {0, -996, 15}, {0, -996, 0}, WHITE);
    }

    void EquipmentSystem::GenerateRenderTexture(entt::entity entity, float width, float height)
    {
        initRenderTextureScene();
        auto& equipment = registry->get<EquipmentComponent>(entity);

        auto& transform = registry->get<sgTransform>(entity);
        auto& renderable = registry->get<Renderable>(entity);
        auto& animation = registry->get<Animation>(entity);
        auto oldPos = transform.GetWorldPos();
        auto cameraPos = gameData->camera->GetPosition();
        auto cameraTarget = gameData->camera->getRaylibCam()->target;

        transform.SetPosition({0, -1000, 0});
        gameData->camera->SetCamera({0, -996, 10}, {0, -996, 0});

        UnloadTexture(equipment.renderTexture.texture);
        equipment.renderTexture = LoadRenderTexture(width, height);

        BeginTextureMode(equipment.renderTexture);
        ClearBackground(BLACK);
        BeginMode3D(*gameData->camera->getRaylibCam());
        renderable.GetModel()->Draw(transform.GetWorldPos(), transform.GetScale().x, WHITE);
        // Need weapon model
        EndMode3D();
        EndTextureMode();

        transform.SetPosition(oldPos);
        gameData->camera->SetCamera(cameraPos, cameraTarget);
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
        gameData->lightSubSystem->LinkRenderableToLight(weaponEntity);

        auto& weaponTrans = registry->emplace<sgTransform>(weaponEntity, weaponEntity);
        weaponTrans.SetParent(&transform);
        weaponTrans.SetLocalPos(Vector3Zero());
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
        onEquipmentUpdated.publish(owner);
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
        onEquipmentUpdated.publish(owner);
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
            onEquipmentUpdated.publish(owner);
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