//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "Actor.hpp"

#include "raylib.h"
#include "raymath.h"
#include <entt/entt.hpp>

#include "BaseSystem.hpp"
#include "UserInput.hpp"
#include "EventManager.hpp"

namespace sage
{

class ActorMovementSystem : public BaseSystem<Actor>
{
    UserInput* cursor;
    entt::entity controlledActorId{}; // Temporary for now.
    void onCursorClick();
public:
    void MoveToLocation(entt::entity id);
    void PathfindToLocation(entt::entity id);
    void SetControlledActor(entt::entity id);
    ActorMovementSystem(entt::registry* _registry, UserInput* _cursor);

    void PatrolLocations(entt::entity id, const std::vector<Vector3> &patrol);
};

} // sage
