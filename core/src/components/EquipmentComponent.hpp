//
// Created by Steve Wheeler on 08/10/2024.
//

#pragma once

#include "raylib.h"

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
        RenderTexture renderTexture{};
        std::unordered_map<EquipmentSlotName, entt::entity> slots;
        std::unordered_map<EquipmentSlotName, entt::entity> worldModels;

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
            float width = 200;
            float height = 400;
            renderTexture = LoadRenderTexture(width, height);
        }
    };

} // namespace sage
