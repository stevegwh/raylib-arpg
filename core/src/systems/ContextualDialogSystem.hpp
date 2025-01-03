//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace sage
{
    class GameData;

    class ContextualDialogSystem
    {
        std::unordered_map<entt::entity, std::vector<std::string>> dialogTextMap;
        entt::registry* registry;
        GameData* gameData;
        void initContextualDialogsFromDirectory();
        static std::vector<std::string> loadDialogFromFile(const std::string& key);

      public:
        void Update();
        void Draw2D() const;

        ContextualDialogSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
