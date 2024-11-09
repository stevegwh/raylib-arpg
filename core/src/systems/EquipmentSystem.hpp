//
// Created by Steve Wheeler on 08/11/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    enum class EquipmentItem
    {
        HEAD,
        BOOTS,
        CHEST,
        ARMS,
        LEGS,
        LEFTWEP,
        RIGHTWEP
    };

    class EquipmentSystem
    {
        entt::registry* registry;
        GameData* gameData;

      public:
        entt::sigh<void(entt::entity)> onEquipmentUpdated;
        entt::entity EquipItem(entt::entity owner, entt::entity item, EquipmentItem itemType);
        EquipmentSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
