//
// Created by Steve Wheeler on 08/11/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    enum class EquipmentSlotName;

    // Depends on Animation being initialised
    class EquipmentSystem
    {
        entt::registry* registry;
        GameData* gameData;

        // TODO: Make this an independent class
        entt::entity renderTextureSceneLight = entt::null;
        void updateCharacterPreviewPose(entt::entity entity);
        void updateCharacterWeaponPosition(entt::entity owner) const;
        void instantiateWeapon(entt::entity owner, entt::entity itemId, EquipmentSlotName itemType) const;
        void onComponentAdded(entt::entity addedEntity);
        void onComponentRemoved(entt::entity removedEntity);

      public:
        void GenerateRenderTexture(entt::entity entity, float width, float height);
        entt::sigh<void(entt::entity)> onEquipmentUpdated;
        [[nodiscard]] entt::entity GetItem(entt::entity owner, EquipmentSlotName itemType) const;
        void EquipItem(entt::entity owner, entt::entity item, EquipmentSlotName itemType) const;
        void MoveItemToInventory(entt::entity owner, EquipmentSlotName itemType) const;
        void DestroyItem(entt::entity owner, EquipmentSlotName itemType) const;
        [[nodiscard]] bool SwapItems(entt::entity owner, EquipmentSlotName itemType1, EquipmentSlotName itemType2);
        EquipmentSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
