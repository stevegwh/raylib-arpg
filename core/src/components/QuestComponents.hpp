//
// Created by steve on 29/11/2024.
//

#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace sage
{
    class Quest;

    //    FETCH, // Fetch something
    //    KILL,  // Kill something
    //    TALK,  // Talk to someone
    //    GO     // Go to a specific location

    // Attach this to entities that are part of a quest
    struct QuestTaskComponent
    {
        enum class TaskEnum
        {
            FETCH,
            KILL,
            GO,
            TALK
        };
        TaskEnum taskType;
        bool completed = false;
        [[nodiscard]] bool IsComplete()
        {
            return completed;
        }
    };

    class BaseQuestComponent
    {
        std::vector<entt::entity> quests;

      public:
        void AddQuest(entt::entity questId);
        [[nodiscard]] bool HasQuest(entt::entity questId);
    };

    class QuestGiverComponent : public BaseQuestComponent
    {

      public:
    };

    // Attach this to entities who can receive quests
    class QuestReceiverComponent : public BaseQuestComponent
    {

      public:
    };

    struct QuestRewards
    {
        // TODO
    };

    class Quest
    {
        entt::registry* registry{};
        bool completed = false;
        entt::entity questId{};
        entt::entity questGiver = entt::null;
        int experience{};
        std::string journalExplanation;
        std::vector<QuestRewards> rewards;
        std::vector<entt::entity> tasks;

      public:
        [[nodiscard]] bool IsComplete();
        [[nodiscard]] entt::entity GetQuestGiver() const;

        explicit Quest(
            entt::registry* _registry,
            entt::entity _questId,
            entt::entity _questGiver,
            std::vector<entt::entity> _tasks);
    };

} // namespace sage
