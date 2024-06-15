//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "../components/ControllableActor.hpp"

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

class ControllableActorMovementSystem : public BaseSystem<ControllableActor>
{
    Cursor* cursor;
    UserInput* userInput;
    entt::entity controlledActorId{}; // TODO: Right now this system just controls one unit at a time. What if we wanted more?
    NavigationGridSystem* navigationGridSystem{};
    TransformSystem* transformSystem{};
    void onFloorClick(entt::entity entity);
    void onEnemyClick(entt::entity entity);
    void onTargetUpdate(entt::entity target);
    void cancelMovement(entt::entity entity);
public:
    ControllableActorMovementSystem(entt::registry* _registry,
                                    Cursor* _cursor,
                                    UserInput* _userInput,
                                    NavigationGridSystem* _navigationGridSystem,
                                    TransformSystem* _transformSystem);
    void Update();
    void MoveToLocation(entt::entity id);
    void PathfindToLocation(entt::entity id, Vector3 location);
    void SetControlledActor(entt::entity id);
    entt::entity GetControlledActor();

    void PatrolLocations(entt::entity id, const std::vector<Vector3> &patrol);
    
    entt::sigh<void(entt::entity)> onControlledActorChange;
    void Enable();
    void Disable();
};

} // sage
