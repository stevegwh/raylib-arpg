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
        void CreateQuest(entt::registry* _registry, const std::string& key);
        void RemoveQuest(const std::string& key);
        entt::entity GetQuest(const std::string& key) const;

        static QuestManager& GetInstance()
        {
            static QuestManager instance;
            return instance;
        }
    };

} // namespace sage
