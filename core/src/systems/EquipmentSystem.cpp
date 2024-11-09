//
// Created by Steve Wheeler on 08/11/2024.
//

#include "EquipmentSystem.hpp"

namespace sage
{

    entt::entity EquipmentSystem::EquipItem(entt::entity owner, entt::entity item, EquipmentItem itemType)
    {
        return entt::null;
    }

    EquipmentSystem::EquipmentSystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage