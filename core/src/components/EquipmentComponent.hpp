//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <unordered_map>

namespace sage
{
    enum class EquipmentSlotName
    {
        HELM,
        BOOTS,
        CHEST,
        ARMS,
        LEGS,
        BELT,
        LEFTHAND,
        RIGHTHAND,
        AMULET,
        RING1,
        RING2
    };

    struct EquipmentComponent
    {
        std::unordered_map<EquipmentSlotName, entt::entity> slots;

        EquipmentComponent()
        {
            slots[EquipmentSlotName::HELM] = entt::null;
            slots[EquipmentSlotName::BOOTS] = entt::null;
            slots[EquipmentSlotName::CHEST] = entt::null;
            slots[EquipmentSlotName::ARMS] = entt::null;
            slots[EquipmentSlotName::LEGS] = entt::null;
            slots[EquipmentSlotName::BELT] = entt::null;
            slots[EquipmentSlotName::LEFTHAND] = entt::null;
            slots[EquipmentSlotName::RIGHTHAND] = entt::null;
            slots[EquipmentSlotName::AMULET] = entt::null;
            slots[EquipmentSlotName::RING1] = entt::null;
            slots[EquipmentSlotName::RING2] = entt::null;
        }
    };

} // namespace sage
