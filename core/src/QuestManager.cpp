//
// Created by Steve Wheeler on 29/11/2024.
//

#include "QuestManager.hpp"

namespace sage
{

    void QuestManager::CreateQuest(entt::registry* _registry, const std::string& key)
    {
        map.emplace(key, _registry->create());
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