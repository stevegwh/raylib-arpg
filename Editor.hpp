//
// Created by steve on 22/02/2024.
//

#pragma once
#include "Cursor.hpp"
#include "EventCallback.hpp"

namespace sage
{
class Editor
{
    Cursor* cursor;
    void OnCursorClick();
    void OnCollisionHit();
    
public:
    Editor(Cursor* _cursor)
    : cursor(_cursor)
    {
        const std::function<void()> f1 = [p = this] { p->OnCursorClick(); };
        cursor->OnClickEvent->Subscribe(std::make_shared<EventCallback>(f1));
        const std::function<void()> f2 = [p = this] { p->OnCollisionHit(); };
        cursor->OnCollisionHitEvent->Subscribe(std::make_shared<EventCallback>(f2));
        
    }
    // OnClickEvent()
    // OnCursorHit(CollisionInfo collision
    //
    // PlaceModel()
    // SelectModel()
    // DeleteModel()
    // PollInput()

    // Store the majority of functionality in "cursor" in this class, as it will be used for editing the map.

};

}



