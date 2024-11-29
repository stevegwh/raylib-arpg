//
// Created by steve on 11/05/2024.
//

#pragma once

#include "systems/BaseSystem.hpp"

#include "components/DialogComponent.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class GameData;
    class sgTransform;
    class Window;

    class DialogSystem : public BaseSystem
    {
        GameData* gameData;
        Window* dialogWindow{};
        Vector3 oldCamPos{};
        Vector3 oldCamTarget{};
        void progressConversation(const dialog::Conversation* conversation);

      public:
        void StartConversation(const sgTransform& cutscenePose, entt::entity npc);
        void EndConversation();
        dialog::Conversation* GetConversation(entt::entity owner, ConversationID conversationId);
        explicit DialogSystem(entt::registry* registry, GameData* _gameData);
    };
} // namespace sage
