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
            const std::string& dialogOptions) const;

      public:
        void InitDialogFromDirectory();

        DialogFactory(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
