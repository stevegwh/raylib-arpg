//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Event.hpp"
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
        Event<Vector2, Vector2> onWindowUpdate{}; // Old, New

        Event<> keyAPressed{};
        Event<> keyAUp{};
        Event<> keyBPressed{};
        Event<> keyBUp{};
        Event<> keyCPressed{};
        Event<> keyCUp{};
        Event<> keyDPressed{};
        Event<> keyDUp{};
        Event<> keyEPressed{};
        Event<> keyEUp{};
        Event<> keyFPressed{};
        Event<> keyFUp{};
        Event<> keyGPressed{};
        Event<> keyGUp{};
        Event<> keyHPressed{};
        Event<> keyHUp{};
        Event<> keyIPressed{};
        Event<> keyIUp{};
        Event<> keyJPressed{};
        Event<> keyJUp{};
        Event<> keyKPressed{};
        Event<> keyKUp{};
        Event<> keyLPressed{};
        Event<> keyLUp{};
        Event<> keyMPressed{};
        Event<> keyMUp{};
        Event<> keyNPressed{};
        Event<> keyNUp{};
        Event<> keyOPressed{};
        Event<> keyOUp{};
        Event<> keyPPressed{};
        Event<> keyPUp{};
        Event<> keyQPressed{};
        Event<> keyQUp{};
        Event<> keyRPressed{};
        Event<> keyRUp{};
        Event<> keySPressed{};
        Event<> keySUp{};
        Event<> keyTPressed{};
        Event<> keyTUp{};
        Event<> keyUPressed{};
        Event<> keyUUp{};
        Event<> keyVPressed{};
        Event<> keyVUp{};
        Event<> keyWPressed{};
        Event<> keyWUp{};
        Event<> keyXPressed{};
        Event<> keyXUp{};
        Event<> keyYPressed{};
        Event<> keyYUp{};
        Event<> keyZPressed{};
        Event<> keyZUp{};
        Event<> keyEscapePressed{};
        Event<> keyEscapeUp{};
        Event<> keySpacePressed{};
        Event<> keySpaceUp{};
        Event<> keyDeletePressed{};
        Event<> keyDeleteUp{};
        Event<> keyOnePressed{};
        Event<> keyOneUp{};
        Event<> keyTwoPressed{};
        Event<> keyTwoUp{};
        Event<> keyThreePressed{};
        Event<> keyThreeUp{};
        Event<> keyFourPressed{};
        Event<> keyFourUp{};

        void ListenForInput() const;

        UserInput(KeyMapping* _keyMapping, Settings* _settings);
    };
} // namespace sage
