//
// Created by steve on 11/05/2024.
//

#pragma once

#include "components/DialogComponent.hpp"
#include "entt/entt.hpp"

namespace sage
{
    class Systems;
    class sgTransform;
    class Window;

    class DialogSystem
    {
        entt::registry* registry;
        Systems* sys;
        Window* dialogWindow{};
        void endConversation(entt::entity npc) const;
        void progressConversation(const dialog::Conversation* conversation);

      public:
        void StartConversation(const sgTransform& cutscenePose, entt::entity npc);
        dialog::Conversation* GetConversation(entt::entity owner, ConversationID conversationId);
        explicit DialogSystem(entt::registry* registry, Systems* _sys);
    };
} // namespace sage
