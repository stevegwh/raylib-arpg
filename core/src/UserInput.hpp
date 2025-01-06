//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "Cursor.hpp"
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
        std::unique_ptr<Event<Vector2, Vector2>> onWindowUpdate{}; // Old, New

        std::unique_ptr<Event<>> keyAPressed{};
        std::unique_ptr<Event<>> keyAUp{};
        std::unique_ptr<Event<>> keyBPressed{};
        std::unique_ptr<Event<>> keyBUp{};
        std::unique_ptr<Event<>> keyCPressed{};
        std::unique_ptr<Event<>> keyCUp{};
        std::unique_ptr<Event<>> keyDPressed{};
        std::unique_ptr<Event<>> keyDUp{};
        std::unique_ptr<Event<>> keyEPressed{};
        std::unique_ptr<Event<>> keyEUp{};
        std::unique_ptr<Event<>> keyFPressed{};
        std::unique_ptr<Event<>> keyFUp{};
        std::unique_ptr<Event<>> keyGPressed{};
        std::unique_ptr<Event<>> keyGUp{};
        std::unique_ptr<Event<>> keyHPressed{};
        std::unique_ptr<Event<>> keyHUp{};
        std::unique_ptr<Event<>> keyIPressed{};
        std::unique_ptr<Event<>> keyIUp{};
        std::unique_ptr<Event<>> keyJPressed{};
        std::unique_ptr<Event<>> keyJUp{};
        std::unique_ptr<Event<>> keyKPressed{};
        std::unique_ptr<Event<>> keyKUp{};
        std::unique_ptr<Event<>> keyLPressed{};
        std::unique_ptr<Event<>> keyLUp{};
        std::unique_ptr<Event<>> keyMPressed{};
        std::unique_ptr<Event<>> keyMUp{};
        std::unique_ptr<Event<>> keyNPressed{};
        std::unique_ptr<Event<>> keyNUp{};
        std::unique_ptr<Event<>> keyOPressed{};
        std::unique_ptr<Event<>> keyOUp{};
        std::unique_ptr<Event<>> keyPPressed{};
        std::unique_ptr<Event<>> keyPUp{};
        std::unique_ptr<Event<>> keyQPressed{};
        std::unique_ptr<Event<>> keyQUp{};
        std::unique_ptr<Event<>> keyRPressed{};
        std::unique_ptr<Event<>> keyRUp{};
        std::unique_ptr<Event<>> keySPressed{};
        std::unique_ptr<Event<>> keySUp{};
        std::unique_ptr<Event<>> keyTPressed{};
        std::unique_ptr<Event<>> keyTUp{};
        std::unique_ptr<Event<>> keyUPressed{};
        std::unique_ptr<Event<>> keyUUp{};
        std::unique_ptr<Event<>> keyVPressed{};
        std::unique_ptr<Event<>> keyVUp{};
        std::unique_ptr<Event<>> keyWPressed{};
        std::unique_ptr<Event<>> keyWUp{};
        std::unique_ptr<Event<>> keyXPressed{};
        std::unique_ptr<Event<>> keyXUp{};
        std::unique_ptr<Event<>> keyYPressed{};
        std::unique_ptr<Event<>> keyYUp{};
        std::unique_ptr<Event<>> keyZPressed{};
        std::unique_ptr<Event<>> keyZUp{};
        std::unique_ptr<Event<>> keyEscapePressed{};
        std::unique_ptr<Event<>> keyEscapeUp{};
        std::unique_ptr<Event<>> keySpacePressed{};
        std::unique_ptr<Event<>> keySpaceUp{};
        std::unique_ptr<Event<>> keyDeletePressed{};
        std::unique_ptr<Event<>> keyDeleteUp{};
        std::unique_ptr<Event<>> keyOnePressed{};
        std::unique_ptr<Event<>> keyOneUp{};
        std::unique_ptr<Event<>> keyTwoPressed{};
        std::unique_ptr<Event<>> keyTwoUp{};
        std::unique_ptr<Event<>> keyThreePressed{};
        std::unique_ptr<Event<>> keyThreeUp{};
        std::unique_ptr<Event<>> keyFourPressed{};
        std::unique_ptr<Event<>> keyFourUp{};

        void ListenForInput() const;

        UserInput(KeyMapping* _keyMapping, Settings* _settings);
    };
} // namespace sage
