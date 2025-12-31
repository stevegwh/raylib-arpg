//
// Created by steve on 11/05/2024.
//

#pragma once

#include "components/DialogComponent.hpp"
#include "entt/entt.hpp"

namespace sage
{
    class sgTransform;
    class Window;
} // namespace sage

namespace lq
{
    class Systems;

    class DialogSystem
    {
        entt::registry* registry;
        Systems* sys;
        sage::Window* dialogWindow{};
        void endConversation(entt::entity npc) const;
        void progressConversation(const dialog::Conversation* conversation);

      public:
        void StartConversation(const sage::sgTransform& cutscenePose, entt::entity npc);
        dialog::Conversation* GetConversation(entt::entity owner, ConversationID conversationId);
        explicit DialogSystem(entt::registry* registry, Systems* _sys);
    };
} // namespace lq
