//
// Created by Steve Wheeler on 30/11/2024.
//

#include "ItemFactory.hpp"

#include "components/ItemComponent.hpp"

namespace lq
{

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
} // namespace lq