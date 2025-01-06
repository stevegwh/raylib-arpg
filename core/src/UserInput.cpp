//
// Created by Steve Wheeler on 18/02/2024->
//

#include "UserInput.hpp"

namespace sage
{
    void UserInput::toggleFullScreen() const
    {
        settings->toggleFullScreenRequested = true;

        if (!IsWindowFullscreen())
        {
            const int current_screen = GetCurrentMonitor();
            settings->SetScreenSize(GetMonitorWidth(current_screen), GetMonitorHeight(current_screen));
        }
        else if (IsWindowFullscreen())
        {
            settings->ResetToUserDefined();
        }
    }

    void UserInput::ListenForInput() const
    {
        if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
        {
            toggleFullScreen();
        }

        if (IsKeyPressed(keyMapping->keyA))
        {
            keyAPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyA))
        {
            keyAUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyB))
        {
            keyBPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyB))
        {
            keyBUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyC))
        {
            keyCPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyC))
        {
            keyCUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyD))
        {
            keyDPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyD))
        {
            keyDUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyE))
        {
            keyEPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyE))
        {
            keyEUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyF))
        {
            keyFPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyF))
        {
            keyFUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyG))
        {
            keyGPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyG))
        {
            keyGUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyH))
        {
            keyHPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyH))
        {
            keyHUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyI))
        {
            keyIPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyI))
        {
            keyIUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyJ))
        {
            keyJPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyJ))
        {
            keyJUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyK))
        {
            keyKPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyK))
        {
            keyKUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyL))
        {
            keyLPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyL))
        {
            keyLUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyM))
        {
            keyMPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyM))
        {
            keyMUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyN))
        {
            keyNPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyN))
        {
            keyNUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyO))
        {
            keyOPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyO))
        {
            keyOUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyP))
        {
            keyPPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyP))
        {
            keyPUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyQ))
        {
            keyQPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyQ))
        {
            keyQUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyR))
        {
            keyRPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyR))
        {
            keyRUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyS))
        {
            keySPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyS))
        {
            keySUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyT))
        {
            keyTPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyT))
        {
            keyTUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyU))
        {
            keyUPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyU))
        {
            keyUUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyV))
        {
            keyVPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyV))
        {
            keyVUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyW))
        {
            keyWPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyW))
        {
            keyWUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyX))
        {
            keyXPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyX))
        {
            keyXUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyY))
        {
            keyYPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyY))
        {
            keyYUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyZ))
        {
            keyZPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyZ))
        {
            keyZUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyEscape))
        {
            keyEscapePressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyEscape))
        {
            keyEscapeUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keySpace))
        {
            keySpacePressed->Publish();
        }
        if (IsKeyUp(keyMapping->keySpace))
        {
            keySpaceUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyDelete))
        {
            keyDeletePressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyDelete))
        {
            keyDeleteUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyOne))
        {
            keyOnePressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyOne))
        {
            keyOneUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyTwo))
        {
            keyTwoPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyTwo))
        {
            keyTwoUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyThree))
        {
            keyThreePressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyThree))
        {
            keyThreeUp->Publish();
        }
        if (IsKeyPressed(keyMapping->keyFour))
        {
            keyFourPressed->Publish();
        }
        if (IsKeyUp(keyMapping->keyFour))
        {
            keyFourUp->Publish();
        }
    }

    UserInput::UserInput(KeyMapping* _keyMapping, Settings* _settings)
        : keyMapping(_keyMapping),
          settings(_settings),
          onWindowUpdate(std::make_unique<Event<Vector2, Vector2>>()),
          keyAPressed(std::make_unique<Event<>>()),
          keyAUp(std::make_unique<Event<>>()),
          keyBPressed(std::make_unique<Event<>>()),
          keyBUp(std::make_unique<Event<>>()),
          keyCPressed(std::make_unique<Event<>>()),
          keyCUp(std::make_unique<Event<>>()),
          keyDPressed(std::make_unique<Event<>>()),
          keyDUp(std::make_unique<Event<>>()),
          keyEPressed(std::make_unique<Event<>>()),
          keyEUp(std::make_unique<Event<>>()),
          keyFPressed(std::make_unique<Event<>>()),
          keyFUp(std::make_unique<Event<>>()),
          keyGPressed(std::make_unique<Event<>>()),
          keyGUp(std::make_unique<Event<>>()),
          keyHPressed(std::make_unique<Event<>>()),
          keyHUp(std::make_unique<Event<>>()),
          keyIPressed(std::make_unique<Event<>>()),
          keyIUp(std::make_unique<Event<>>()),
          keyJPressed(std::make_unique<Event<>>()),
          keyJUp(std::make_unique<Event<>>()),
          keyKPressed(std::make_unique<Event<>>()),
          keyKUp(std::make_unique<Event<>>()),
          keyLPressed(std::make_unique<Event<>>()),
          keyLUp(std::make_unique<Event<>>()),
          keyMPressed(std::make_unique<Event<>>()),
          keyMUp(std::make_unique<Event<>>()),
          keyNPressed(std::make_unique<Event<>>()),
          keyNUp(std::make_unique<Event<>>()),
          keyOPressed(std::make_unique<Event<>>()),
          keyOUp(std::make_unique<Event<>>()),
          keyPPressed(std::make_unique<Event<>>()),
          keyPUp(std::make_unique<Event<>>()),
          keyQPressed(std::make_unique<Event<>>()),
          keyQUp(std::make_unique<Event<>>()),
          keyRPressed(std::make_unique<Event<>>()),
          keyRUp(std::make_unique<Event<>>()),
          keySPressed(std::make_unique<Event<>>()),
          keySUp(std::make_unique<Event<>>()),
          keyTPressed(std::make_unique<Event<>>()),
          keyTUp(std::make_unique<Event<>>()),
          keyUPressed(std::make_unique<Event<>>()),
          keyUUp(std::make_unique<Event<>>()),
          keyVPressed(std::make_unique<Event<>>()),
          keyVUp(std::make_unique<Event<>>()),
          keyWPressed(std::make_unique<Event<>>()),
          keyWUp(std::make_unique<Event<>>()),
          keyXPressed(std::make_unique<Event<>>()),
          keyXUp(std::make_unique<Event<>>()),
          keyYPressed(std::make_unique<Event<>>()),
          keyYUp(std::make_unique<Event<>>()),
          keyZPressed(std::make_unique<Event<>>()),
          keyZUp(std::make_unique<Event<>>()),
          keyEscapePressed(std::make_unique<Event<>>()),
          keyEscapeUp(std::make_unique<Event<>>()),
          keySpacePressed(std::make_unique<Event<>>()),
          keySpaceUp(std::make_unique<Event<>>()),
          keyDeletePressed(std::make_unique<Event<>>()),
          keyDeleteUp(std::make_unique<Event<>>()),
          keyOnePressed(std::make_unique<Event<>>()),
          keyOneUp(std::make_unique<Event<>>()),
          keyTwoPressed(std::make_unique<Event<>>()),
          keyTwoUp(std::make_unique<Event<>>()),
          keyThreePressed(std::make_unique<Event<>>()),
          keyThreeUp(std::make_unique<Event<>>()),
          keyFourPressed(std::make_unique<Event<>>()),
          keyFourUp(std::make_unique<Event<>>())
    {
        assert(settings != nullptr);
        assert(keyMapping != nullptr);
    }
} // namespace sage
