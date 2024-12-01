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

        template <class Archive>
        void save(Archive& archive) const
        {
            std::vector<ItemComponent> items;
            auto view = registry->view<ItemComponent>();
            for (auto entity : view)
            {
                auto& item = registry->get<ItemComponent>(entity);
                items.push_back(item);
            }

            archive(cereal::make_nvp("Items", items));
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive();
        }

        // TODO: Change this to use JSON files that act as "recipes" for the items. Store the recipe by their UUID
        // and retrieve them with GetItem(std::string uuid). Don't use an enum.

        ItemFactory(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
