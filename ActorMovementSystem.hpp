//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Actor.hpp"

#include "raylib.h"
#include "raymath.h"

#include "BaseSystem.hpp"
#include "UserInput.hpp"
#include "EventManager.hpp"

namespace sage
{

class ActorMovementSystem : public BaseSystem<Actor>
{
    UserInput* cursor;
    EntityID actorId; // Temporary for now.
    
    void onCursorClick();
public:
    
    void SetControlledActor(EntityID id);
    explicit ActorMovementSystem(UserInput* _cursor) : cursor(_cursor) {}
    
};

} // sage
