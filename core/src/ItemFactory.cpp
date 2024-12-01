//
// Created by Steve Wheeler on 30/11/2024.
//

#include "ItemFactory.hpp"

#include "components/ItemComponent.hpp"

namespace sage
{
    void ItemFactory::CreateDagger()
    {
        ItemComponent item;
        item.name = "Dagger";
        item.description = "A test inventory item.";
        item.icon = AssetID::IMG_ICON_WEAPON_DAGGER01;
        item.model = AssetID::MDL_WPN_DAGGER01;
        item.AddFlag(ItemFlags::WEAPON | ItemFlags::DAGGER);
        itemMap.emplace(item.name, item);
    }

    void ItemFactory::CreateSword()
    {
        ItemComponent item;
        item.name = "Sword";
        item.description = "A test inventory item.";
        item.icon = AssetID::IMG_ICON_WEAPON_SWORD01;
        item.model = AssetID::MDL_WPN_SWORD01;
        item.AddFlag(ItemFlags::WEAPON | ItemFlags::SWORD | ItemFlags::MAIN_HAND_ONLY);
        itemMap.emplace(item.name, item);
    }

    entt::entity ItemFactory::GetItem(const std::string& name) const
    {
        auto entity = registry->create();
        auto item = itemMap.at(name);
        registry->emplace<ItemComponent>(entity, item);
        return entity;
    }

    ItemFactory::ItemFactory(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
    }
} // namespace sage