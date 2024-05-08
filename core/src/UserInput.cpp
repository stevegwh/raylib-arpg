//
// Created by Steve Wheeler on 18/02/2024->
//

#include "UserInput.hpp"

namespace sage
{

void UserInput::toggleFullScreen() const
{
    if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        if (!IsWindowFullscreen())
        {
            const int current_screen = GetCurrentMonitor();
            SetWindowSize(
                GetMonitorWidth(current_screen),
                GetMonitorHeight(current_screen));
            ToggleFullscreen();
        }
        else if (IsWindowFullscreen())
        {
            ToggleFullscreen();
            SetWindowSize(settings->SCREEN_WIDTH, settings->SCREEN_HEIGHT);
        };
    }
}

void UserInput::ListenForInput() const
{
    toggleFullScreen();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (dOnClickEvent) dOnClickEvent();
    }

    if (IsKeyPressed(keyMapping->keyA))
    {
        if (dKeyAPressed) dKeyAPressed();
    }
    if (IsKeyUp(keyMapping->keyA))
    {
        if (dKeyAUp) dKeyAUp();
    }
    if (IsKeyPressed(keyMapping->keyB))
    {
        if (dKeyBPressed) dKeyBPressed();
    }
    if (IsKeyUp(keyMapping->keyB))
    {
        if (dKeyBUp) dKeyBUp();
    }
    if (IsKeyPressed(keyMapping->keyC))
    {
        if (dKeyCPressed) dKeyCPressed();
    }
    if (IsKeyUp(keyMapping->keyC))
    {
        if (dKeyCUp) dKeyCUp();
    }
    if (IsKeyPressed(keyMapping->keyD))
    {
        if (dKeyDPressed) dKeyDPressed();
    }
    if (IsKeyUp(keyMapping->keyD))
    {
        if (dKeyDUp) dKeyDUp();
    }
    if (IsKeyPressed(keyMapping->keyE))
    {
        if (dKeyEPressed) dKeyEPressed();
    }
    if (IsKeyUp(keyMapping->keyE))
    {
        if (dKeyEUp) dKeyEUp();
    }
    if (IsKeyPressed(keyMapping->keyF))
    {
        if (dKeyFPressed) dKeyFPressed();
    }
    if (IsKeyUp(keyMapping->keyF))
    {
        if (dKeyFUp) dKeyFUp();
    }
    if (IsKeyPressed(keyMapping->keyG))
    {
        if (dKeyGPressed) dKeyGPressed();
    }
    if (IsKeyUp(keyMapping->keyG))
    {
        if (dKeyGUp) dKeyGUp();
    }
    if (IsKeyPressed(keyMapping->keyH))
    {
        if (dKeyHPressed) dKeyHPressed();
    }
    if (IsKeyUp(keyMapping->keyH))
    {
        if (dKeyHUp) dKeyHUp();
    }
    if (IsKeyPressed(keyMapping->keyI))
    {
        if (dKeyIPressed) dKeyIPressed();
    }
    if (IsKeyUp(keyMapping->keyI))
    {
        if (dKeyIUp) dKeyIUp();
    }
    if (IsKeyPressed(keyMapping->keyJ))
    {
        if (dKeyJPressed) dKeyJPressed();
    }
    if (IsKeyUp(keyMapping->keyJ))
    {
        if (dKeyJUp) dKeyJUp();
    }
    if (IsKeyPressed(keyMapping->keyK))
    {
        if (dKeyKPressed) dKeyKPressed();
    }
    if (IsKeyUp(keyMapping->keyK))
    {
        if (dKeyKUp) dKeyKUp();
    }
    if (IsKeyPressed(keyMapping->keyL))
    {
        if (dKeyLPressed) dKeyLPressed();
    }
    if (IsKeyUp(keyMapping->keyL))
    {
        if (dKeyLUp) dKeyLUp();
    }
    if (IsKeyPressed(keyMapping->keyM))
    {
        if (dKeyMPressed) dKeyMPressed();
    }
    if (IsKeyUp(keyMapping->keyM))
    {
        if (dKeyMUp) dKeyMUp();
    }
    if (IsKeyPressed(keyMapping->keyN))
    {
        if (dKeyNPressed) dKeyNPressed();
    }
    if (IsKeyUp(keyMapping->keyN))
    {
        if (dKeyNUp) dKeyNUp();
    }
    if (IsKeyPressed(keyMapping->keyO))
    {
        if (dKeyOPressed) dKeyOPressed();
    }
    if (IsKeyUp(keyMapping->keyO))
    {
        if (dKeyOUp) dKeyOUp();
    }
    if (IsKeyPressed(keyMapping->keyP))
    {
        if (dKeyPPressed) dKeyPPressed();
    }
    if (IsKeyUp(keyMapping->keyP))
    {
        if (dKeyPUp) dKeyPUp();
    }
    if (IsKeyPressed(keyMapping->keyQ))
    {
        if (dKeyQPressed) dKeyQPressed();
    }
    if (IsKeyUp(keyMapping->keyQ))
    {
        if (dKeyQUp) dKeyQUp();
    }
    if (IsKeyPressed(keyMapping->keyR))
    {
        if (dKeyRPressed) dKeyRPressed();
    }
    if (IsKeyUp(keyMapping->keyR))
    {
        if (dKeyRUp) dKeyRUp();
    }
    if (IsKeyPressed(keyMapping->keyS))
    {
        if (dKeySPressed) dKeySPressed();
    }
    if (IsKeyUp(keyMapping->keyS))
    {
        if (dKeySUp) dKeySUp();
    }
    if (IsKeyPressed(keyMapping->keyT))
    {
        if (dKeyTPressed) dKeyTPressed();
    }
    if (IsKeyUp(keyMapping->keyT))
    {
        if (dKeyTUp) dKeyTUp();
    }
    if (IsKeyPressed(keyMapping->keyU))
    {
        if (dKeyUPressed) dKeyUPressed();
    }
    if (IsKeyUp(keyMapping->keyU))
    {
        if (dKeyUUp) dKeyUUp();
    }
    if (IsKeyPressed(keyMapping->keyV))
    {
        if (dKeyVPressed) dKeyVPressed();
    }
    if (IsKeyUp(keyMapping->keyV))
    {
        if (dKeyVUp) dKeyVUp();
    }
    if (IsKeyPressed(keyMapping->keyW))
    {
        if (dKeyWPressed) dKeyWPressed();
    }
    if (IsKeyUp(keyMapping->keyW))
    {
        if (dKeyWUp) dKeyWUp();
    }
    if (IsKeyPressed(keyMapping->keyX))
    {
        if (dKeyXPressed) dKeyXPressed();
    }
    if (IsKeyUp(keyMapping->keyX))
    {
        if (dKeyXUp) dKeyXUp();
    }
    if (IsKeyPressed(keyMapping->keyY))
    {
        if (dKeyYPressed) dKeyYPressed();
    }
    if (IsKeyUp(keyMapping->keyY))
    {
        if (dKeyYUp) dKeyYUp();
    }
    if (IsKeyPressed(keyMapping->keyZ))
    {
        if (dKeyZPressed) dKeyZPressed();
    }
    if (IsKeyUp(keyMapping->keyZ))
    {
        if (dKeyZUp) dKeyZUp();
    }
    if (IsKeyPressed(keyMapping->keyEscape))
    {
        if (dKeyEscapePressed) dKeyEscapePressed();
    }
    if (IsKeyUp(keyMapping->keyEscape))
    {
        if (dKeyEscapeUp) dKeyEscapeUp();
    }
    if (IsKeyPressed(keyMapping->keySpace))
    {
        if (dKeySpacePressed) dKeySpacePressed();
    }
    if (IsKeyUp(keyMapping->keySpace))
    {
        if (dKeySpaceUp) dKeySpaceUp();
    }
    if (IsKeyPressed(keyMapping->keyDelete))
    {
        if (dKeyDeletePressed) dKeyDeletePressed();
    }
    if (IsKeyUp(keyMapping->keyDelete))
    {
        if (dKeyDeleteUp) dKeyDeleteUp();
    }

}
    
UserInput::UserInput(KeyMapping* _keyMapping, Settings* _settings) : 
keyMapping(_keyMapping), settings(_settings) {}

}
