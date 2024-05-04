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
    entt::entity controlledActorId{}; // Temporary for now.
    NavigationGridSystem* navigationGridSystem{};
    TransformSystem* transformSystem{};
    void onCursorClick();
public:
    void MoveToLocation(entt::entity id);
    void PathfindToLocation(entt::entity id);
    void SetControlledActor(entt::entity id);
    ActorMovementSystem(entt::registry* _registry,
                         Cursor* _cursor,
                         UserInput* _userInput,
                         NavigationGridSystem* _navigationGridSystem,
                         TransformSystem* _transformSystem);

    void PatrolLocations(entt::entity id, const std::vector<Vector3> &patrol);
};

} // sage
