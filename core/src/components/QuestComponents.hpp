//
// Created by steve on 29/11/2024.
//

#pragma once

#include "Event.hpp"

#include "entt/entt.hpp"

#include <iostream>
#include <utility>
#include <vector>

namespace sage
{
    class Quest;

    // Attach this to entities that are part of a quest
    struct QuestTaskComponent
    {
        std::string questKey;
        bool completed = false;
        // entt::sigh<void(QuestTaskComponent*)> onTaskCompleted;
        std::unique_ptr<Event<QuestTaskComponent*>> onTaskCompleted;

        void MarkComplete()
        {
            std::cout << "Task complete! \n";
            completed = true;
            onTaskCompleted->Publish(this);
        }

        [[nodiscard]] bool IsComplete() const
        {
            return completed;
        }

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(questKey);
        }

        explicit QuestTaskComponent(std::string _questKey)
            : questKey(std::move(_questKey)), onTaskCompleted(std::make_unique<Event<QuestTaskComponent*>>())
        {
        }
        QuestTaskComponent() = default;
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
        std::string questKey{};
        entt::entity questId{};
        std::vector<entt::entity> tasks;

      public:
        std::string journalTitle;
        std::string journalDescription;
        [[nodiscard]] unsigned int GetTaskCount() const;
        [[nodiscard]] unsigned int GetTaskCompleteCount() const;
        void StartQuest();
        void AddTask(entt::entity taskId);
        void CompleteQuest();
        [[nodiscard]] bool AllTasksComplete() const;
        [[nodiscard]] bool IsComplete() const;
        [[nodiscard]] bool HasStarted() const;

        std::unique_ptr<Event<entt::entity>> onQuestStart;
        std::unique_ptr<Event<entt::entity>> onQuestCompleted;

        explicit Quest(entt::registry* _registry, entt::entity _questId, std::string _questKey);
    };

} // namespace sage
