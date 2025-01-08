//
// Created by Steve Wheeler on 29/11/2024.
//

#pragma once

#include "entt/entt.hpp"

#include <string>
#include <unordered_map>

namespace sage
{
    struct Systems;

    class QuestManager
    {
        entt::registry* registry;
        Systems* sys;
        std::unordered_map<std::string, entt::entity> map{};
        entt::entity createQuest(const std::string& key);

      public:
        void InitQuestsFromDirectory();
        void RemoveQuest(const std::string& key);
        [[nodiscard]] entt::entity GetQuest(const std::string& key) const;

        explicit QuestManager(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
