//
// Created by Steve Wheeler on 08/11/2024.
//

#pragma once

#include "Event.hpp"

#include "entt/entt.hpp"
// #include <memory>

namespace sage
{
    class Systems;
    enum class EquipmentSlotName;

    // Depends on Animation being initialised
    class EquipmentSystem
    {
        entt::registry* registry;
        Systems* sys;

        // TODO: Make this an independent class
        entt::entity renderTextureSceneLight = entt::null;
        void updateCharacterPreviewPose(entt::entity entity);
        void updateCharacterWeaponPosition(entt::entity owner) const;
        void instantiateWeapon(entt::entity owner, entt::entity itemId, EquipmentSlotName itemType) const;
        void onComponentAdded(entt::entity addedEntity);
        void onComponentRemoved(entt::entity removedEntity);

      public:
        void GeneratePortraitRenderTexture(entt::entity entity, float width, float height);
        void GenerateRenderTexture(entt::entity entity, float width, float height);
        Event<entt::entity> onEquipmentUpdated; // Selected actor
        [[nodiscard]] entt::entity GetItem(entt::entity owner, EquipmentSlotName itemType) const;
        void EquipItem(entt::entity owner, entt::entity item, EquipmentSlotName itemType) const;
        void MoveItemToInventory(entt::entity owner, EquipmentSlotName itemType) const;
        void DestroyItem(entt::entity owner, EquipmentSlotName itemType) const;
        [[nodiscard]] bool SwapItems(entt::entity owner, EquipmentSlotName itemType1, EquipmentSlotName itemType2);
        EquipmentSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
