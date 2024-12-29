//
// Created by Steve Wheeler on 29/11/2024.
//

#pragma once

#include <entt/entt.hpp>
#include <string>

namespace sage
{
    class GameData;

    namespace dialog
    {
        class Conversation;
    } // namespace dialog

    class DialogFactory
    {
        entt::registry* registry;
        GameData* gameData;

        void parseNode(
            dialog::Conversation* conversation,
            const std::string& nodeName,
            const std::string& content,
            const std::vector<std::vector<std::string>>& optionData);

      public:
        void LoadDialog();

        DialogFactory(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
