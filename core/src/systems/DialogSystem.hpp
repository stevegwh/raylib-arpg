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
    class sgTransform;
    class Window;

    class DialogSystem : public BaseSystem
    {
        GameData* gameData;
        Window* dialogWindow{};

        Vector3 oldCamPos{};
        Vector3 oldCamTarget{};

        // void NPCClicked(entt::entity _clickedNPC);
        // void cancelConversation(entt::entity entity);
        // void startConversation(entt::entity actor);
        // void endConversation(entt::entity actor);

      public:
        void StartConversation(const sgTransform& cutscenePose, entt::entity npc);
        void EndConversation();
        explicit DialogSystem(entt::registry* registry, GameData* _gameData);
    };
} // namespace sage
