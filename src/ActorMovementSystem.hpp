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
    EntityID controlledActorId; // Temporary for now.
    void onCursorClick();
public:
    void MoveToLocation(EntityID id);
    void PathfindToLocation(EntityID id);
    void SetControlledActor(EntityID id);
    explicit ActorMovementSystem(UserInput* _cursor) : cursor(_cursor) {}

    void PatrolLocations(EntityID id, const std::vector<Vector3> &patrol);
};

} // sage
