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
        item.localizedName = "Worn Dagger";
        item.description = "A worn and slightly pathetic dagger.";
        item.icon = "IMG_ICON_WEAPON_DAGGER01";
        item.model = "MDL_WPN_DAGGER01";
        item.AddFlag(ItemFlags::WEAPON | ItemFlags::DAGGER);
        itemMap.emplace(item.name, item);
    }

    void ItemFactory::CreateSword()
    {
        ItemComponent item;
        item.name = "Sword";
        item.name = "Common Sword";
        item.description = "A test inventory item.";
        item.icon = "IMG_ICON_WEAPON_SWORD01";
        item.model = "MDL_WPN_SWORD01";
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

    void ItemFactory::AttachItem(entt::entity entity, const std::string& name) const
    {
        auto item = itemMap.at(name);
        registry->emplace<ItemComponent>(entity, item);
    }

    ItemFactory::ItemFactory(entt::registry* _registry) : registry(_registry)
    {
    }
} // namespace sage