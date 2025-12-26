//
// Created by Steve Wheeler on 29/11/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "Event.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace sage
{
    struct Systems;
    class Quest;

    class QuestManager
    {
        entt::registry* registry;
        Systems* sys;
        std::unordered_map<std::string, entt::entity> map{};
        std::unordered_map<entt::entity, std::vector<Subscription>> connectionMap{};
        entt::entity createQuest(const std::string& key);
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        Event<entt::entity> onQuestUpdate{};
        void InitQuestsFromDirectory();
        void RemoveQuest(const std::string& key);
        std::vector<Quest*> GetActiveQuests();
        [[nodiscard]] entt::entity GetQuest(const std::string& key) const;

        explicit QuestManager(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
