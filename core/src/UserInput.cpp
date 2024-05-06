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

    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
    {
        if (IsKeyPressed(keyMapping.keyS))
        {
            if (dOnSerializeSaveKeyPressedEvent) dOnSerializeSaveKeyPressedEvent();
        }
        else if (IsKeyPressed(keyMapping.keyL))
        {
            if (dOnSerializeLoadKeyPressedEvent) dOnSerializeLoadKeyPressedEvent();
        }
    }
    else
    {
        if (IsKeyPressed(keyMapping.keyDelete))
        {
            if (dOnDeleteKeyPressedEvent) dOnDeleteKeyPressedEvent();
        }
        else if (IsKeyPressed(keyMapping.keyP))
        {
            if(dOnCreateKeyPressedEvent) dOnCreateKeyPressedEvent();
        }
        else if (IsKeyPressed(keyMapping.keyG))
        {
            if (dOnGenGridKeyPressedEvent) dOnGenGridKeyPressedEvent();
        }
        else if (IsKeyPressed(keyMapping.keyR))
        {
            if (dOnRunModePressedEvent) dOnRunModePressedEvent();
        }
    }

}
    
UserInput::UserInput(Cursor* _cursor, KeyMapping _keyMapping) : cursor(_cursor), keyMapping(_keyMapping)
{
    
}

}
