//
// Created by Steve Wheeler on 18/02/2024.
//

#include "UserInput.hpp"

namespace sage
{

void UserInput::ListenForInput()
{
    if (cursor->GetMouseRayCollision())
    {
        if (dOnCollisionHitEvent) dOnCollisionHitEvent(); 
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (dOnClickEvent) dOnClickEvent();
    }

    if (IsKeyPressed(keyMapping.keyA))
    {
        if (dKeyAPressed) dKeyAPressed();
    }
    else if (IsKeyPressed(keyMapping.keyB))
    {
        if (dKeyBPressed) dKeyBPressed();
    }
    else if (IsKeyPressed(keyMapping.keyC))
    {
        if (dKeyCPressed) dKeyCPressed();
    }
    else if (IsKeyPressed(keyMapping.keyD))
    {
        if (dKeyDPressed) dKeyDPressed();
    }
    else if (IsKeyPressed(keyMapping.keyE))
    {
        if (dKeyEPressed) dKeyEPressed();
    }
    else if (IsKeyPressed(keyMapping.keyF))
    {
        if (dKeyFPressed) dKeyFPressed();
    }
    else if (IsKeyPressed(keyMapping.keyG))
    {
        if (dKeyGPressed) dKeyGPressed();
    }
    else if (IsKeyPressed(keyMapping.keyH))
    {
        if (dKeyHPressed) dKeyHPressed();
    }
    else if (IsKeyPressed(keyMapping.keyI))
    {
        if (dKeyIPressed) dKeyIPressed();
    }
    else if (IsKeyPressed(keyMapping.keyJ))
    {
        if (dKeyJPressed) dKeyJPressed();
    }
    else if (IsKeyPressed(keyMapping.keyK))
    {
        if (dKeyKPressed) dKeyKPressed();
    }
    else if (IsKeyPressed(keyMapping.keyL))
    {
        if (dKeyLPressed) dKeyLPressed();
    }
    else if (IsKeyPressed(keyMapping.keyM))
    {
        if (dKeyMPressed) dKeyMPressed();
    }
    else if (IsKeyPressed(keyMapping.keyN))
    {
        if (dKeyNPressed) dKeyNPressed();
    }
    else if (IsKeyPressed(keyMapping.keyO))
    {
        if (dKeyOPressed) dKeyOPressed();
    }
    else if (IsKeyPressed(keyMapping.keyP))
    {
        if (dKeyPPressed) dKeyPPressed();
    }
    else if (IsKeyPressed(keyMapping.keyQ))
    {
        if (dKeyQPressed) dKeyQPressed();
    }
    else if (IsKeyPressed(keyMapping.keyR))
    {
        if (dKeyRPressed) dKeyRPressed();
    }
    else if (IsKeyPressed(keyMapping.keyS))
    {
        if (dKeySPressed) dKeySPressed();
    }
    else if (IsKeyPressed(keyMapping.keyT))
    {
        if (dKeyTPressed) dKeyTPressed();
    }
    else if (IsKeyPressed(keyMapping.keyU))
    {
        if (dKeyUPressed) dKeyUPressed();
    }
    else if (IsKeyPressed(keyMapping.keyV))
    {
        if (dKeyVPressed) dKeyVPressed();
    }
    else if (IsKeyPressed(keyMapping.keyW))
    {
        if (dKeyWPressed) dKeyWPressed();
    }
    else if (IsKeyPressed(keyMapping.keyX))
    {
        if (dKeyXPressed) dKeyXPressed();
    }
    else if (IsKeyPressed(keyMapping.keyY))
    {
        if (dKeyYPressed) dKeyYPressed();
    }
    else if (IsKeyPressed(keyMapping.keyZ))
    {
        if (dKeyZPressed) dKeyZPressed();
    }
    else if (IsKeyPressed(keyMapping.keyEscape))
    {
        if (dKeyEscapePressed) dKeyEscapePressed();
    }
    else if (IsKeyPressed(keyMapping.keySpace))
    {
        if (dKeySpacePressed) dKeySpacePressed();
    }
    else if (IsKeyPressed(keyMapping.keyDelete))
    {
        if (dKeyDeletePressed) dKeyDeletePressed();
    }

}
    
UserInput::UserInput(Cursor* _cursor, KeyMapping _keyMapping) : cursor(_cursor), keyMapping(_keyMapping)
{
    
}

}
