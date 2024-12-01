//
// Created by Steve Wheeler on 30/11/2024.
//

#include "ItemFactory.hpp"

#include "components/ItemComponent.hpp"

namespace sage
{
    // TODO: These *needs* to be replaced with something data driven. Read from JSON.
    // Instead of an enum as an ID we can use a string, I guess. Maybe a better option exists.

    entt::entity CreateDagger(entt::registry* registry)
    {
        auto itemEntity = registry->create();
        auto& item = registry->emplace<ItemComponent>(itemEntity);
        item.name = "Dagger";
        item.description = "A test inventory item.";
        item.icon = AssetID::IMG_ICON_WEAPON_DAGGER01;
        item.model = AssetID::MDL_WPN_DAGGER01;
        item.AddFlag(ItemFlags::WEAPON | ItemFlags::DAGGER);
        return itemEntity;
    }

    entt::entity CreateSword(entt::registry* registry)
    {
        auto itemEntity = registry->create();
        auto& item = registry->emplace<ItemComponent>(itemEntity);
        item.name = "Sword";
        item.description = "A test inventory item.";
        item.icon = AssetID::IMG_ICON_WEAPON_SWORD01;
        item.model = AssetID::MDL_WPN_SWORD01;
        item.AddFlag(ItemFlags::WEAPON | ItemFlags::SWORD);
        return itemEntity;
    }

    entt::entity ItemFactory::GetItem(ItemID itemId) const
    {
        if (itemId == ItemID::DAGGER)
        {
            return CreateDagger(registry);
        }
        else if (itemId == ItemID::SWORD)
        {
            return CreateSword(registry);
        }
        return entt::null;
    }

    ItemFactory::ItemFactory(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage