//
// Created by Steve Wheeler on 29/11/2024.
//

#pragma once

#include <entt/entt.hpp>

#include <string>
#include <unordered_map>

namespace sage
{
    struct GameData;

    class QuestManager
    {
        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<std::string, entt::entity> map{};
        entt::entity createQuest(const std::string& key);

      public:
        void InitQuestsFromDirectory();
        void RemoveQuest(const std::string& key);
        entt::entity GetQuest(const std::string& key);
        void AddTaskToQuest(const std::string& questKey, entt::entity taskId);
        void AddTaskToQuest(entt::entity questId, entt::entity taskId);

        explicit QuestManager(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
