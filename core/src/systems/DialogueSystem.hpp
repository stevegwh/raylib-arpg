//
// Created by steve on 11/05/2024.
//

#pragma once

#include "raylib.h"
#include "systems/BaseSystem.hpp"

#include "entt/entt.hpp"

namespace sage
{
    class GameData;
    class Window;
    class DialogueSystem : public BaseSystem
    {
        GameData* gameData;
        std::unique_ptr<Window> window;
        bool active = false;
        entt::entity selectedActor;
        entt::entity clickedNPC;

        Vector3 oldCamPos{};
        Vector3 oldCamTarget{};

        void NPCClicked(entt::entity _clickedNPC);
        void changeControlledActor(entt::entity entity);
        void cancelConversation(entt::entity entity);
        void startConversation(entt::entity actor);
        void endConversation(entt::entity actor);

      public:
        entt::sigh<void()> onConversationStart;
        entt::sigh<void()> onConversationEnd;

        explicit DialogueSystem(entt::registry* registry, GameData* _gameData);
    };
} // namespace sage
