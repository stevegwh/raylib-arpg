//
// Created by Steve Wheeler on 08/11/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    enum class EquipmentType;

    class EquipmentSystem
    {
        entt::registry* registry;
        GameData* gameData;

      public:
        entt::sigh<void(entt::entity)> onEquipmentUpdated;
        [[nodiscard]] entt::entity GetItem(entt::entity owner, EquipmentType itemType) const;
        void EquipItem(entt::entity owner, entt::entity item, EquipmentType itemType);
        void UnequipItem(entt::entity owner, EquipmentType itemType) const;
        EquipmentSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
