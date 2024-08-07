//
// Created by steve on 11/05/2024.
//

#pragma once

#include "DialogueWindow.hpp"
#include "systems/BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class GameData;
    class DialogueSystem : public BaseSystem
    {
        GameData* gameData;
        bool active = false;
        entt::entity controlledActor;
        entt::entity clickedNPC;

        Vector3 oldCamPos;
        Vector3 oldCamTarget;
        std::unique_ptr<DialogueWindow> window;

        void NPCClicked(entt::entity _clickedNPC);
        void changeControlledActor(entt::entity entity);
        void cancelConversation(entt::entity entity);
        void startConversation(entt::entity actor);
        void endConversation(entt::entity actor);

      public:
        explicit DialogueSystem(entt::registry* registry, GameData* _gameData);
        entt::sigh<void()> onConversationStart;
        entt::sigh<void()> onConversationEnd;

        void Update();
        void Draw2D();
    };
} // namespace sage
