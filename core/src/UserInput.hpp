//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Cursor.hpp"
#include "KeyMapping.hpp"
#include "Settings.hpp"

#include "raylib.h"

namespace sage
{
    class UserInput
    {
        KeyMapping* keyMapping;
        Settings* settings;
        void toggleFullScreen() const;

      public:
        entt::sigh<void(Vector2, Vector2)> onWindowUpdate{}; // Old, New

        entt::sigh<void()> keyAPressed{};
        entt::sigh<void()> keyAUp{};
        entt::sigh<void()> keyBPressed{};
        entt::sigh<void()> keyBUp{};
        entt::sigh<void()> keyCPressed{};
        entt::sigh<void()> keyCUp{};
        entt::sigh<void()> keyDPressed{};
        entt::sigh<void()> keyDUp{};
        entt::sigh<void()> keyEPressed{};
        entt::sigh<void()> keyEUp{};
        entt::sigh<void()> keyFPressed{};
        entt::sigh<void()> keyFUp{};
        entt::sigh<void()> keyGPressed{};
        entt::sigh<void()> keyGUp{};
        entt::sigh<void()> keyHPressed{};
        entt::sigh<void()> keyHUp{};
        entt::sigh<void()> keyIPressed{};
        entt::sigh<void()> keyIUp{};
        entt::sigh<void()> keyJPressed{};
        entt::sigh<void()> keyJUp{};
        entt::sigh<void()> keyKPressed{};
        entt::sigh<void()> keyKUp{};
        entt::sigh<void()> keyLPressed{};
        entt::sigh<void()> keyLUp{};
        entt::sigh<void()> keyMPressed{};
        entt::sigh<void()> keyMUp{};
        entt::sigh<void()> keyNPressed{};
        entt::sigh<void()> keyNUp{};
        entt::sigh<void()> keyOPressed{};
        entt::sigh<void()> keyOUp{};
        entt::sigh<void()> keyPPressed{};
        entt::sigh<void()> keyPUp{};
        entt::sigh<void()> keyQPressed{};
        entt::sigh<void()> keyQUp{};
        entt::sigh<void()> keyRPressed{};
        entt::sigh<void()> keyRUp{};
        entt::sigh<void()> keySPressed{};
        entt::sigh<void()> keySUp{};
        entt::sigh<void()> keyTPressed{};
        entt::sigh<void()> keyTUp{};
        entt::sigh<void()> keyUPressed{};
        entt::sigh<void()> keyUUp{};
        entt::sigh<void()> keyVPressed{};
        entt::sigh<void()> keyVUp{};
        entt::sigh<void()> keyWPressed{};
        entt::sigh<void()> keyWUp{};
        entt::sigh<void()> keyXPressed{};
        entt::sigh<void()> keyXUp{};
        entt::sigh<void()> keyYPressed{};
        entt::sigh<void()> keyYUp{};
        entt::sigh<void()> keyZPressed{};
        entt::sigh<void()> keyZUp{};
        entt::sigh<void()> keyEscapePressed{};
        entt::sigh<void()> keyEscapeUp{};
        entt::sigh<void()> keySpacePressed{};
        entt::sigh<void()> keySpaceUp{};
        entt::sigh<void()> keyDeletePressed{};
        entt::sigh<void()> keyDeleteUp{};
        entt::sigh<void()> keyOnePressed{};
        entt::sigh<void()> keyOneUp{};
        entt::sigh<void()> keyTwoPressed{};
        entt::sigh<void()> keyTwoUp{};
        entt::sigh<void()> keyThreePressed{};
        entt::sigh<void()> keyThreeUp{};
        entt::sigh<void()> keyFourPressed{};
        entt::sigh<void()> keyFourUp{};

        void ListenForInput() const;

        UserInput(KeyMapping* _keyMapping, Settings* _settings);
    };
} // namespace sage
