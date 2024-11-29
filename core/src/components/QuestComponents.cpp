//
// Created by steve on 29/11/2024.
//

#include "QuestComponents.hpp"

#include <utility>

namespace sage
{

    void BaseQuestComponent::AddQuest(entt::entity questId)
    {
        if (!HasQuest(questId))
        {
            quests.push_back(questId);
        }
    }

    bool BaseQuestComponent::HasQuest(entt::entity questId)
    {
        for (const auto& entity : quests)
        {
            if (questId == entity) return true;
        }
        return false;
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
        completed = true;
        return true;
    }

    entt::entity Quest::GetQuestGiver() const
    {
        return questGiver;
    }

    Quest::Quest(
        entt::registry* _registry,
        entt::entity _questID,
        entt::entity _questGiver,
        std::vector<entt::entity> _tasks)
        : registry(_registry), questId(_questID), questGiver(_questGiver), tasks(std::move(_tasks))
    {
    }
} // namespace sage