//
// Created by Steve Wheeler on 08/11/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    enum class EquipmentSlotName;

    class EquipmentSystem
    {
        entt::registry* registry;
        GameData* gameData;

        std::unordered_map<EquipmentSlotName, entt::entity> worldModels;

        void instantiateWeapon(entt::entity owner, entt::entity itemId, EquipmentSlotName itemType);

      public:
        entt::sigh<void(entt::entity)> onEquipmentUpdated;
        [[nodiscard]] entt::entity GetItem(entt::entity owner, EquipmentSlotName itemType) const;
        void EquipItem(entt::entity owner, entt::entity item, EquipmentSlotName itemType);
        void UnequipItem(entt::entity owner, EquipmentSlotName itemType);
        [[nodiscard]] bool SwapItems(
            entt::entity owner,
            entt::entity item1,
            EquipmentSlotName itemType1,
            entt::entity item2,
            EquipmentSlotName itemType2);
        EquipmentSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
