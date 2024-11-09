//
// Created by Steve Wheeler on 08/11/2024.
//

#include "EquipmentSystem.hpp"

#include "components/EquipmentComponent.hpp"
#include "components/Renderable.hpp"
#include "components/WeaponComponent.hpp"
#include "GameData.hpp"
#include "ResourceManager.hpp"
#include "slib.hpp"
#include "systems/LightSubSystem.hpp"

#include "components/ItemComponent.hpp"
#include "components/sgTransform.hpp"
#include "raylib.h"
#include "raymath.h"

namespace sage
{

    void EquipmentSystem::instantiateWeapon(entt::entity owner, entt::entity itemId)
    {
        auto weaponEntity = registry->create();
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

    void EquipmentSystem::EquipItem(entt::entity owner, entt::entity item, EquipmentSlotName itemType)
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        equipment.slots[itemType] = item;
        if (itemType == EquipmentSlotName::LEFTHAND)
        {
            instantiateWeapon(owner, item);
        }
        onEquipmentUpdated.publish(owner);
    }

    void EquipmentSystem::UnequipItem(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        equipment.slots[itemType] = entt::null;
        onEquipmentUpdated.publish(owner);
    }

    bool EquipmentSystem::SwapItems(
        entt::entity owner,
        entt::entity item1,
        EquipmentSlotName itemType1,
        entt::entity item2,
        EquipmentSlotName itemType2)
    {
        if ((itemType1 == EquipmentSlotName::RING1 || itemType1 == EquipmentSlotName::RING2) &&
            (itemType2 == EquipmentSlotName::RING1 || itemType2 == EquipmentSlotName::RING2))
        {
        }
        if ((itemType1 == EquipmentSlotName::LEFTHAND || itemType1 == EquipmentSlotName::RIGHTHAND) &&
            (itemType2 == EquipmentSlotName::LEFTHAND || itemType2 == EquipmentSlotName::RIGHTHAND))
        {
        }
    }

    EquipmentSystem::EquipmentSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage