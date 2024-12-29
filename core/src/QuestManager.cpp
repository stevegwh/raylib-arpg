//
// Created by Steve Wheeler on 29/11/2024.
//

#include "QuestManager.hpp"
#include "components/QuestComponents.hpp"

namespace sage
{

    entt::entity QuestManager::createQuest(const std::string& key)
    {
        auto entity = registry->create();
        registry->emplace<Quest>(entity, registry, entity, key);
        map.emplace(key, entity);

        return entity;
    }

    void QuestManager::RemoveQuest(const std::string& key)
    {
        map.erase(key);
        // TODO: Surely remove from registry?
    }

    entt::entity QuestManager::GetQuest(const std::string& key)
    {
        // assert(map.contains(key));
        if (!map.contains(key))
        {
            return createQuest(key);
        }
        return map.at(key);
    }

    QuestManager::QuestManager(entt::registry* _registry) : registry(_registry)
    {
    }

} // namespace sage