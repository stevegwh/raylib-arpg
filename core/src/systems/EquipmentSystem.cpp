//
// Created by Steve Wheeler on 08/11/2024.
//

#include "EquipmentSystem.hpp"

#include "components/EquipmentComponent.hpp"

namespace sage
{

    entt::entity EquipmentSystem::GetItem(entt::entity owner, EquipmentType itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (itemType == EquipmentType::HELM)
        {
            return equipment.helm;
        }
        if (itemType == EquipmentType::ARMS)
        {
            return equipment.arms;
        }
        if (itemType == EquipmentType::LEGS)
        {
            return equipment.legs;
        }
        if (itemType == EquipmentType::BOOTS)
        {
            return equipment.boots;
        }
        if (itemType == EquipmentType::CHEST)
        {
            return equipment.chest;
        }
        if (itemType == EquipmentType::BELT)
        {
            return equipment.belt;
        }
        if (itemType == EquipmentType::LEFTHAND)
        {
            return equipment.leftHand;
        }
        if (itemType == EquipmentType::RIGHTHAND)
        {
            return equipment.rightHand;
        }
        if (itemType == EquipmentType::AMULET)
        {
            return equipment.amulet;
        }
        if (itemType == EquipmentType::RING1)
        {
            return equipment.ring1;
        }
        if (itemType == EquipmentType::RING2)
        {
            return equipment.ring2;
        }
        return entt::null;
    }

    void EquipmentSystem::EquipItem(entt::entity owner, entt::entity item, EquipmentType itemType)
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (itemType == EquipmentType::HELM)
        {
            equipment.helm = item;
        }
        else if (itemType == EquipmentType::ARMS)
        {
            equipment.arms = item;
        }
        else if (itemType == EquipmentType::LEGS)
        {
            equipment.legs = item;
        }
        else if (itemType == EquipmentType::BOOTS)
        {
            equipment.boots = item;
        }
        else if (itemType == EquipmentType::CHEST)
        {
            equipment.chest = item;
        }
        else if (itemType == EquipmentType::BELT)
        {
            equipment.belt = item;
        }
        else if (itemType == EquipmentType::LEFTHAND)
        {
            equipment.leftHand = item;
        }
        else if (itemType == EquipmentType::RIGHTHAND)
        {
            equipment.rightHand = item;
        }
        else if (itemType == EquipmentType::AMULET)
        {
            equipment.amulet = item;
        }
        else if (itemType == EquipmentType::RING1)
        {
            equipment.ring1 = item;
        }
        else if (itemType == EquipmentType::RING2)
        {
            equipment.ring2 = item;
        }

        onEquipmentUpdated.publish(owner);
    }

    void EquipmentSystem::UnequipItem(entt::entity owner, EquipmentType itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (itemType == EquipmentType::HELM)
        {
            equipment.helm = entt::null;
        }
        else if (itemType == EquipmentType::ARMS)
        {
            equipment.arms = entt::null;
        }
        else if (itemType == EquipmentType::LEGS)
        {
            equipment.legs = entt::null;
        }
        else if (itemType == EquipmentType::BOOTS)
        {
            equipment.boots = entt::null;
        }
        else if (itemType == EquipmentType::CHEST)
        {
            equipment.chest = entt::null;
        }
        else if (itemType == EquipmentType::BELT)
        {
            equipment.belt = entt::null;
        }
        else if (itemType == EquipmentType::LEFTHAND)
        {
            equipment.leftHand = entt::null;
        }
        else if (itemType == EquipmentType::RIGHTHAND)
        {
            equipment.rightHand = entt::null;
        }
        else if (itemType == EquipmentType::AMULET)
        {
            equipment.amulet = entt::null;
        }
        else if (itemType == EquipmentType::RING1)
        {
            equipment.ring1 = entt::null;
        }
        else if (itemType == EquipmentType::RING2)
        {
            equipment.ring2 = entt::null;
        }
        onEquipmentUpdated.publish(owner);
    }

    EquipmentSystem::EquipmentSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage