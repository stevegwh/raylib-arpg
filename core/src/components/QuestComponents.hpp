//
// Created by steve on 29/11/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <iostream>

#include <vector>

namespace sage
{
    class Quest;

    // Attach this to entities that are part of a quest
    struct QuestTaskComponent
    {
        bool completed = false;
        entt::sigh<void(QuestTaskComponent*)> onTaskCompleted;

        void MarkComplete()
        {
            std::cout << "Task complete! \n";
            completed = true;
            onTaskCompleted.publish(this);
        }

        [[nodiscard]] bool IsComplete() const
        {
            return completed;
        }

        explicit QuestTaskComponent(entt::registry* _registry)
        {
        }
    };

    struct QuestRewards
    {
        // TODO
    };

    class Quest
    {
        entt::registry* registry{};
        bool started = false;
        bool completed = false;
        entt::entity questId{};
        int experience{};
        std::string journalExplanation;
        std::vector<QuestRewards> rewards;
        std::vector<entt::entity> tasks;

      public:
        [[nodiscard]] unsigned int GetTaskCount() const;
        [[nodiscard]] unsigned int GetTaskCompleteCount() const;
        void StartQuest();
        void AddTask(entt::entity taskId);
        [[nodiscard]] bool IsComplete();
        [[nodiscard]] bool HasStarted() const;

        entt::sigh<void(entt::entity)> onQuestStart;
        entt::sigh<void(entt::entity)> onQuestCompleted;

        explicit Quest(entt::registry* _registry, entt::entity _questId);
    };

} // namespace sage
