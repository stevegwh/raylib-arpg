//
// Created by steve on 29/11/2024.
//

#include "QuestComponents.hpp"

#include <iostream>

namespace sage
{

    bool Quest::HasStarted() const
    {
        return started;
    }

    void Quest::AddTask(entt::entity taskId)
    {
        tasks.push_back(taskId);
        auto& taskComponent = registry->get<QuestTaskComponent>(taskId);
        entt::sink sink{taskComponent.onTaskCompleted};
        sink.connect<&Quest::IsComplete>(this);
    }

    unsigned int Quest::GetTaskCount() const
    {
        return tasks.size();
    }

    unsigned int Quest::GetTaskCompleteCount() const
    {
        unsigned int count = 0;
        for (const auto& task : tasks)
        {
            auto& taskComponent = registry->get<QuestTaskComponent>(task);
            if (taskComponent.IsComplete())
            {
                ++count;
            }
        }
        return count;
    }

    void Quest::StartQuest()
    {
        std::cout << "Quest started! \n";
        started = true;
        onQuestStart.publish(questId);
    }

    bool Quest::IsComplete()
    {
        if (completed)
        {
            return completed;
        }
        for (const auto& entity : tasks)
        {
            auto& subQuest = registry->get<QuestTaskComponent>(entity);
            if (!subQuest.IsComplete()) return false;
        }
        std::cout << "Quest complete! \n";
        completed = true;
        onQuestCompleted.publish(questId);
        return true;
    }

    Quest::Quest(entt::registry* _registry, entt::entity _questID) : registry(_registry), questId(_questID)
    {
    }
} // namespace sage