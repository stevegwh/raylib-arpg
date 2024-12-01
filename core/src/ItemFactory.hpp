//
// Created by Steve Wheeler on 30/11/2024.
//

#pragma once

#include "components/ItemComponent.hpp"

#include "cereal/cereal.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/core/type_traits.hpp"
#include <cereal/archives/json.hpp>

#include <magic_enum.hpp>
#include <unordered_map>
#include <vector>

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
        std::unordered_map<entt::entity, std::unordered_map<ItemID, entt::entity>> itemMap;

      public:
        [[nodiscard]] entt::entity GetItem(ItemID itemId) const;

        ItemFactory(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
