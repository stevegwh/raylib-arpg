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

    if (IsKeyPressed(KEY_DELETE))
    {
        if (dOnDeleteKeyPressedEvent) dOnDeleteKeyPressedEvent();
    }
    else if (IsKeyPressed(KEY_P))
    {
        if(dOnCreateKeyPressedEvent) dOnCreateKeyPressedEvent();
    } 
    else if (IsKeyPressed(KEY_G))
    {
        if (dOnGenGridKeyPressedEvent) dOnGenGridKeyPressedEvent();
    }
    else if (IsKeyPressed(KEY_Z))
    {
        if (dOnSerializeKeyPressedEvent) dOnSerializeKeyPressedEvent();
    }
    else if (IsKeyPressed(KEY_R))
    {
        if (dOnRunModePressedEvent) dOnRunModePressedEvent();
    }
}
    
UserInput::UserInput(Cursor* _cursor) : cursor(_cursor)
{
    
}

}
