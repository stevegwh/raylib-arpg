//
// Created by Steve Wheeler on 29/11/2024.
//

#include "QuestManager.hpp"
#include "components/QuestComponents.hpp"

namespace sage
{

    entt::entity QuestManager::CreateQuest(entt::registry* _registry, const std::string& key)
    {
        auto entity = _registry->create();
        _registry->emplace<Quest>(entity, _registry, entity);
        map.emplace(key, entity);

        return entity;
    }

    void QuestManager::RemoveQuest(const std::string& key)
    {
        map.erase(key);
    }

    entt::entity QuestManager::GetQuest(const std::string& key) const
    {
        assert(map.contains(key));
        return map.at(key);
    }

} // namespace sage