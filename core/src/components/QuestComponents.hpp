//
// Created by steve on 29/11/2024.
//

#pragma once

#include <entt/entt.hpp>
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

        template <class Archive>
        void serialize(Archive& archive)
        {
            archive(questKey);
        }

        explicit QuestTaskComponent(std::string _questKey) : questKey(std::move(_questKey))
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

        entt::sigh<void(entt::entity)> onQuestStart;
        entt::sigh<void(entt::entity)> onQuestCompleted;

        explicit Quest(entt::registry* _registry, entt::entity _questId, std::string _questKey);
    };

    // Attach this if you want something to happen to a specific entity once a quest is complete
    // TODO: Sure you can't just use a hook? Maybe two sinks? One if we care about the quest (give reward) one if
    // not?
    class QuestEventReactionComponent
    {
        entt::connection completedConnection;
        entt::connection startConnection;
        entt::entity self{};

        void reactToQuestFinish(entt::entity) const
        {
            onQuestCompleted.publish(self);
        }

        void reactToQuestStart(entt::entity) const
        {
            onQuestStart.publish(self);
        }

      public:
        ~QuestEventReactionComponent()
        {
            if (completedConnection)
            {
                completedConnection.release();
            }
            if (startConnection)
            {
                startConnection.release();
            }
        }

        entt::sigh<void(entt::entity)> onQuestStart;
        entt::sigh<void(entt::entity)> onQuestCompleted;
        QuestEventReactionComponent(entt::entity _self, Quest& quest) : self(_self)
        {
            {
                entt::sink sink{quest.onQuestCompleted};
                completedConnection = sink.connect<&QuestEventReactionComponent::reactToQuestFinish>(this);
            }
            {
                entt::sink sink{quest.onQuestStart};
                startConnection = sink.connect<&QuestEventReactionComponent::reactToQuestStart>(this);
            }
        }
    };

} // namespace sage
