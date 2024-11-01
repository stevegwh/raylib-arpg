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
            settings->screenWidth = static_cast<float>(GetMonitorWidth(current_screen));
            settings->screenHeight = static_cast<float>(GetMonitorHeight(current_screen));
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
            keyAPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyA))
        {
            keyAUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyB))
        {
            keyBPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyB))
        {
            keyBUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyC))
        {
            keyCPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyC))
        {
            keyCUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyD))
        {
            keyDPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyD))
        {
            keyDUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyE))
        {
            keyEPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyE))
        {
            keyEUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyF))
        {
            keyFPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyF))
        {
            keyFUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyG))
        {
            keyGPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyG))
        {
            keyGUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyH))
        {
            keyHPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyH))
        {
            keyHUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyI))
        {
            keyIPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyI))
        {
            keyIUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyJ))
        {
            keyJPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyJ))
        {
            keyJUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyK))
        {
            keyKPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyK))
        {
            keyKUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyL))
        {
            keyLPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyL))
        {
            keyLUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyM))
        {
            keyMPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyM))
        {
            keyMUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyN))
        {
            keyNPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyN))
        {
            keyNUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyO))
        {
            keyOPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyO))
        {
            keyOUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyP))
        {
            keyPPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyP))
        {
            keyPUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyQ))
        {
            keyQPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyQ))
        {
            keyQUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyR))
        {
            keyRPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyR))
        {
            keyRUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyS))
        {
            keySPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyS))
        {
            keySUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyT))
        {
            keyTPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyT))
        {
            keyTUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyU))
        {
            keyUPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyU))
        {
            keyUUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyV))
        {
            keyVPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyV))
        {
            keyVUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyW))
        {
            keyWPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyW))
        {
            keyWUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyX))
        {
            keyXPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyX))
        {
            keyXUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyY))
        {
            keyYPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyY))
        {
            keyYUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyZ))
        {
            keyZPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyZ))
        {
            keyZUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyEscape))
        {
            keyEscapePressed.publish();
        }
        if (IsKeyUp(keyMapping->keyEscape))
        {
            keyEscapeUp.publish();
        }
        if (IsKeyPressed(keyMapping->keySpace))
        {
            keySpacePressed.publish();
        }
        if (IsKeyUp(keyMapping->keySpace))
        {
            keySpaceUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyDelete))
        {
            keyDeletePressed.publish();
        }
        if (IsKeyUp(keyMapping->keyDelete))
        {
            keyDeleteUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyOne))
        {
            keyOnePressed.publish();
        }
        if (IsKeyUp(keyMapping->keyOne))
        {
            keyOneUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyTwo))
        {
            keyTwoPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyTwo))
        {
            keyTwoUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyThree))
        {
            keyThreePressed.publish();
        }
        if (IsKeyUp(keyMapping->keyThree))
        {
            keyThreeUp.publish();
        }
        if (IsKeyPressed(keyMapping->keyFour))
        {
            keyFourPressed.publish();
        }
        if (IsKeyUp(keyMapping->keyFour))
        {
            keyFourUp.publish();
        }
    }

    UserInput::UserInput(KeyMapping* _keyMapping, Settings* _settings)
        : keyMapping(_keyMapping), settings(_settings)
    {
        assert(settings != nullptr);
        assert(keyMapping != nullptr);
    }
} // namespace sage
