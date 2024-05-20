//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "../components/Actor.hpp"

#include "raylib.h"
#include "raymath.h"
#include "entt/entt.hpp"

#include "BaseSystem.hpp"
#include "../Cursor.hpp"
#include "../UserInput.hpp"
#include "TransformSystem.hpp"
#include "NavigationGridSystem.hpp"

namespace sage
{

class ActorMovementSystem : public BaseSystem<Actor>
{
    Cursor* cursor;
    UserInput* userInput;
    entt::entity controlledActorId{}; // TODO: Right now this system just controls one unit at a time. What if we wanted more?
    NavigationGridSystem* navigationGridSystem{};
    TransformSystem* transformSystem{};
    void onCursorClick();
public:
    ActorMovementSystem(entt::registry* _registry,
                        Cursor* _cursor,
                        UserInput* _userInput,
                        NavigationGridSystem* _navigationGridSystem,
                        TransformSystem* _transformSystem);
    void MoveToLocation(entt::entity id);
    void PathfindToLocation(entt::entity id, Vector3 location);
    void SetControlledActor(entt::entity id);
    entt::entity GetControlledActor();

    void PatrolLocations(entt::entity id, const std::vector<Vector3> &patrol);
    
    entt::sigh<void(entt::entity)> onControlledActorChange;
    void EnableMovement();
    void DisableMovement();
};

} // sage
