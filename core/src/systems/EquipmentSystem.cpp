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
        if (itemType == EquipmentSlotName::HELM)
        {
            return equipment.helm;
        }
        if (itemType == EquipmentSlotName::ARMS)
        {
            return equipment.arms;
        }
        if (itemType == EquipmentSlotName::LEGS)
        {
            return equipment.legs;
        }
        if (itemType == EquipmentSlotName::BOOTS)
        {
            return equipment.boots;
        }
        if (itemType == EquipmentSlotName::CHEST)
        {
            return equipment.chest;
        }
        if (itemType == EquipmentSlotName::BELT)
        {
            return equipment.belt;
        }
        if (itemType == EquipmentSlotName::LEFTHAND)
        {
            return equipment.leftHand;
        }
        if (itemType == EquipmentSlotName::RIGHTHAND)
        {
            return equipment.rightHand;
        }
        if (itemType == EquipmentSlotName::AMULET)
        {
            return equipment.amulet;
        }
        if (itemType == EquipmentSlotName::RING1)
        {
            return equipment.ring1;
        }
        if (itemType == EquipmentSlotName::RING2)
        {
            return equipment.ring2;
        }
        return entt::null;
    }

    void EquipmentSystem::EquipItem(entt::entity owner, entt::entity item, EquipmentSlotName itemType)
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (itemType == EquipmentSlotName::HELM)
        {
            equipment.helm = item;
        }
        else if (itemType == EquipmentSlotName::ARMS)
        {
            equipment.arms = item;
        }
        else if (itemType == EquipmentSlotName::LEGS)
        {
            equipment.legs = item;
        }
        else if (itemType == EquipmentSlotName::BOOTS)
        {
            equipment.boots = item;
        }
        else if (itemType == EquipmentSlotName::CHEST)
        {
            equipment.chest = item;
        }
        else if (itemType == EquipmentSlotName::BELT)
        {
            equipment.belt = item;
        }
        else if (itemType == EquipmentSlotName::LEFTHAND)
        {
            equipment.leftHand = item;
            instantiateWeapon(owner, item);
        }
        else if (itemType == EquipmentSlotName::RIGHTHAND)
        {
            equipment.rightHand = item;
            instantiateWeapon(owner, item);
        }
        else if (itemType == EquipmentSlotName::AMULET)
        {
            equipment.amulet = item;
        }
        else if (itemType == EquipmentSlotName::RING1)
        {
            equipment.ring1 = item;
        }
        else if (itemType == EquipmentSlotName::RING2)
        {
            equipment.ring2 = item;
        }

        onEquipmentUpdated.publish(owner);
    }

    void EquipmentSystem::UnequipItem(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (itemType == EquipmentSlotName::HELM)
        {
            equipment.helm = entt::null;
        }
        else if (itemType == EquipmentSlotName::ARMS)
        {
            equipment.arms = entt::null;
        }
        else if (itemType == EquipmentSlotName::LEGS)
        {
            equipment.legs = entt::null;
        }
        else if (itemType == EquipmentSlotName::BOOTS)
        {
            equipment.boots = entt::null;
        }
        else if (itemType == EquipmentSlotName::CHEST)
        {
            equipment.chest = entt::null;
        }
        else if (itemType == EquipmentSlotName::BELT)
        {
            equipment.belt = entt::null;
        }
        else if (itemType == EquipmentSlotName::LEFTHAND)
        {
            equipment.leftHand = entt::null;
        }
        else if (itemType == EquipmentSlotName::RIGHTHAND)
        {
            equipment.rightHand = entt::null;
        }
        else if (itemType == EquipmentSlotName::AMULET)
        {
            equipment.amulet = entt::null;
        }
        else if (itemType == EquipmentSlotName::RING1)
        {
            equipment.ring1 = entt::null;
        }
        else if (itemType == EquipmentSlotName::RING2)
        {
            equipment.ring2 = entt::null;
        }
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