//
// Created by Steve Wheeler on 29/11/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <unordered_map>

namespace sage
{

    class QuestManager
    {
        std::unordered_map<std::string, entt::entity> map{};
        QuestManager() = default;
        ~QuestManager() = default;

      public:
        entt::entity CreateQuest(entt::registry* _registry, const std::string& key);
        void RemoveQuest(const std::string& key);
        [[nodiscard]] entt::entity GetQuest(const std::string& key) const;

        static QuestManager& GetInstance()
        {
            static QuestManager instance;
            return instance;
        }

        // TODO: Move renderable.name to its own component and create a system that has a map of Name ->
        // std::vector<entt::entity>. This identifier component can also specify if it must be unique, then the
        // system will assert(size <= 1) to enforce it. This would allow quests to specify a quest giver by a named
        // id (rather than an enum).
        //
        // Likewise, ItemFactory should use strings instead of an enum. Moving to string
        // identifiers allows us to reference these things consistently in JSON files without having to add
        // anything to a C++ enum (and recompiling).
    };

} // namespace sage
