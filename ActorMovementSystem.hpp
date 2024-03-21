//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Actor.hpp"

#include "raylib.h"
#include "raymath.h"
#include "BaseSystem.hpp"
#include "UserInput.hpp"

namespace sage
{

class ActorMovementSystem : public BaseSystem<Actor>
{
    UserInput* cursor;
    EntityID playerId; // Temporary for now.
    
    void onCursorClick();
public:

    explicit ActorMovementSystem(UserInput* _cursor, EntityID _playerId) : BaseSystem<Actor>("Actor"), cursor(_cursor), playerId(_playerId)
    {
        const std::function<void()> f1 = [p = this] { p->onCursorClick(); };
        cursor->OnClickEvent->Subscribe(std::make_shared<EventCallback>(f1));
    }


};

} // sage
