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

    class QuestTaskType
    {
      protected:
        entt::registry* registry;
        // entt::entity questId;
        entt::entity questTarget;

        explicit QuestTaskType(entt::registry* _registry, entt::entity _questTarget)
            : registry(_registry), questTarget(_questTarget)
        {
        }

      public:
        [[nodiscard]] virtual bool CheckComplete()
        {
            return false;
        };
        virtual ~QuestTaskType() = default;
    };

    struct FetchQuest : public QuestTaskType
    {
        // Get what?
        [[nodiscard]] bool CheckComplete() override
        {
            return false;
        }

        explicit FetchQuest(entt::registry* _registry, entt::entity _questTarget)
            : QuestTaskType(_registry, _questTarget)
        {
        }
    };

    struct TalkQuest : public QuestTaskType
    {
        // With who?
        [[nodiscard]] bool CheckComplete() override
        {
            return false;
        }

        explicit TalkQuest(entt::registry* _registry, entt::entity _questTarget)
            : QuestTaskType(_registry, _questTarget)
        {
        }
    };

    struct KillQuest : public QuestTaskType
    {
        // Kill who?
        [[nodiscard]] bool CheckComplete() override
        {
            return false;
        }

        explicit KillQuest(entt::registry* _registry, entt::entity _questTarget)
            : QuestTaskType(_registry, _questTarget)
        {
        }
    };

    // Attach this to entities that are part of a quest
    struct QuestTaskComponent
    {
        std::unique_ptr<QuestTaskType> taskType;
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
            return completed || taskType->CheckComplete();
        }
        explicit QuestTaskComponent(entt::registry* _registry, std::unique_ptr<QuestTaskType> _taskType)
            : taskType(std::move(_taskType))
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
        void StartQuest();
        void AddTask(entt::entity taskId);
        [[nodiscard]] bool IsComplete();
        [[nodiscard]] bool HasStarted() const;

        entt::sigh<void(entt::entity)> onQuestStart;
        entt::sigh<void(entt::entity)> onQuestCompleted;

        explicit Quest(entt::registry* _registry, entt::entity _questId);
    };

} // namespace sage
