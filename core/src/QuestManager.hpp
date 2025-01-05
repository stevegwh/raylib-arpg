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
    enum class EventType;
    class Quest;

    class QuestManager
    {
        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<std::string, entt::entity> map{};
        entt::entity createQuest(const std::string& key);
        void bindFunctionToQuestEvent(
            const std::string& functionName, const std::string& functionParams, Quest* quest, EventType eventType);

      public:
        void InitQuestsFromDirectory();
        void RemoveQuest(const std::string& key);
        entt::entity GetQuest(const std::string& key);
        // void AddTaskToQuest(const std::string& questKey, entt::entity taskId);
        // void AddTaskToQuest(entt::entity questId, entt::entity taskId);

        explicit QuestManager(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
