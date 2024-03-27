//
// Created by steve on 22/02/2024.
//

#pragma once
#include "UserInput.hpp"
#include "EventCallback.hpp"

namespace sage
{

enum EditorMode
{
    IDLE,
    SELECT,
    MOVE,
    CREATE
};

// NB: "GameManager" is friend
class Editor
{
    EditorMode currentEditorMode;
    UserInput* cursor;
    void OnCursorClick();
    void OnCollisionHit();
    void OnSerializeButton();
    EntityID selectedObject;
    void moveSelectedObjectToCursorHit();
    
public:
    Editor(UserInput* _cursor)
    : cursor(_cursor)
    {
        const std::function<void()> f1 = [p = this] { p->OnCursorClick(); };
        cursor->OnClickEvent->Subscribe(std::make_shared<EventCallback>(f1));
        const std::function<void()> f2 = [p = this] { p->OnCollisionHit(); };
        cursor->OnCollisionHitEvent->Subscribe(std::make_shared<EventCallback>(f2));
        const std::function<void()> f3 = [p = this] { p->OnDeleteModeKeyPressed(); };
        cursor->OnDeleteKeyPressedEvent->Subscribe(std::make_shared<EventCallback>(f3));
        const std::function<void()> f4 = [p = this] { p->OnCreateModeKeyPressed(); };
        cursor->OnCreateKeyPressedEvent->Subscribe(std::make_shared<EventCallback>(f4));
        const std::function<void()> f5 = [p = this] { p->OnGenGridKeyPressed(); };
        cursor->OnGenGridKeyPressedEvent->Subscribe(std::make_shared<EventCallback>(f5));
        const std::function<void()> f6 = [p = this] { p->OnSerializeButton(); };
        cursor->OnSerializeKeyPressedEvent->Subscribe(std::make_shared<EventCallback>(f6));
    }

    void Draw();
    void DrawDebugText();
    // OnClickEvent()
    // OnCursorHit(CollisionInfo collision
    //
    // PlaceModel()
    // SelectModel()
    // DeleteModel()
    // PollInput()

    // Store the majority of functionality in "cursor" in this class, as it will be used for editing the map.

    void OnDeleteModeKeyPressed();
    void OnCreateModeKeyPressed();
    void OnGenGridKeyPressed();
};

}



