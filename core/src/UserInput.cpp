//
// Created by Steve Wheeler on 18/02/2024.
//

#include "UserInput.hpp"
#include <iostream>

#include "GameManager.hpp"

namespace sage
{

void UserInput::OnClick() const
{
    if (dOnClickEvent) dOnClickEvent();
    //std::cout << "Hit object position: " << collision.point.x << ", " << collision.point.y << ", " << collision.point.z << "\n";
}

void UserInput::OnDeleteKeyPressed() const
{
    if (dOnDeleteKeyPressedEvent) dOnDeleteKeyPressedEvent();
}

void UserInput::OnCreateKeyPressed() const
{
    if(dOnCreateKeyPressedEvent) dOnCreateKeyPressedEvent();
}

void UserInput::OnGenGridKeyPressed() const
{
    if (dOnGenGridKeyPressedEvent) dOnGenGridKeyPressedEvent();
}

void UserInput::OnSerializeKeyPressed() const
{
    if (dOnSerializeKeyPressedEvent) dOnSerializeKeyPressedEvent();
}

void UserInput::ListenForInput()
{
    if (cursor->GetMouseRayCollision())
    {
        if (dOnCollisionHitEvent) dOnCollisionHitEvent(); 
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        OnClick();
    }

    if (IsKeyPressed(KEY_DELETE))
    {
        OnDeleteKeyPressed();
    }
    else if (IsKeyPressed(KEY_P))
    {
        OnCreateKeyPressed();
    } 
    else if (IsKeyPressed(KEY_G))
    {
        OnGenGridKeyPressed();
    }
    else if (IsKeyPressed(KEY_Z))
    {
        OnSerializeKeyPressed();
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
