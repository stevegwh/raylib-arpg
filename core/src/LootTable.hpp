//
// Created by Steve Wheeler on 30/11/2024.
//

#pragma once

#include "components/ItemComponent.hpp"

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"
// #include "cereal/types/string.hpp"
// #include "cereal/types/vector.hpp"
// #include "entt/core/type_traits.hpp"
#include "cereal/archives/json.hpp"

#include <unordered_map>
#include <vector>

namespace sage
{
    class Systems;
    class InventoryComponent;

    class LootTable
    {
        entt::registry* registry;
        Systems* sys;
        void loadItems(const std::unordered_map<std::string, std::vector<std::string>>& lootTable) const;

      public:
        template <class Archive>
        void save(Archive& archive) const
        {
            assert(0); // Do not call this.
            archive("");
        }

        template <class Archive>
        void load(Archive& archive)
        {
            std::unordered_map<std::string, std::vector<std::string>> lootTable;
            archive(lootTable);

            loadItems(lootTable);

            //
        }

        explicit LootTable(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
