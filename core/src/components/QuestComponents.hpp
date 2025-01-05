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
        std::unique_ptr<Event<QuestTaskComponent*>>
            onStart; // NB: nullptr (QuestTasks are not started), defined for compatibility with 'Quest'
        std::unique_ptr<Event<QuestTaskComponent*>> onCompleted;

        void MarkComplete()
        {
            std::cout << "Task complete! \n";
            completed = true;
            onCompleted->Publish(this);
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
            : questKey(std::move(_questKey)), onCompleted(std::make_unique<Event<QuestTaskComponent*>>())
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

        std::unique_ptr<Event<entt::entity>> onStart;
        std::unique_ptr<Event<entt::entity>> onCompleted;

        explicit Quest(entt::registry* _registry, entt::entity _questId, std::string _questKey);
    };

} // namespace sage
