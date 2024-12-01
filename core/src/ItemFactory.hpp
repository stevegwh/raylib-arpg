//
// Created by Steve Wheeler on 30/11/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    enum class ItemID
    {
        DAGGER,
        SWORD
    };

    class ItemFactory
    {
        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<entt::entity, std::unordered_map<ItemID, entt::entity>> abilityMap;

      public:
        entt::entity GetItem(ItemID itemId) const;

        ItemFactory(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
