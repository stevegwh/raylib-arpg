//
// Created by steve on 29/11/2024.
//

#include "QuestComponents.hpp"

#include "QuestManager.hpp"

#include <iostream>
#include <utility>

namespace sage
{

    bool Quest::HasStarted() const
    {
        return started;
    }

    void Quest::AddTask(entt::entity taskId)
    {
        tasks.push_back(taskId);
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
        std::cout << "Quest started: " << questKey << " \n";
        started = true;
        onStart->Publish(questId);
    }

    void Quest::CompleteQuest()
    {
        std::cout << "Quest complete: " << questKey << " \n";
        completed = true;
        onCompleted->Publish(questId);
    }

    bool Quest::AllTasksComplete() const
    {
        for (const auto& entity : tasks)
        {
            auto& subQuest = registry->get<QuestTaskComponent>(entity);
            if (!subQuest.IsComplete()) return false;
        }
        return true;
    }

    bool Quest::IsComplete() const
    {
        assert(started); // Throw error if task has been completed but the quest was not started.
        return completed;
    }

    Quest::Quest(entt::registry* _registry, const entt::entity _questId, std::string _questKey)
        : registry(_registry),
          questKey(std::move(_questKey)),
          questId(_questId),
          onStart(std::make_unique<Event<entt::entity>>()),
          onCompleted(std::make_unique<Event<entt::entity>>())
    {
    }
} // namespace sage