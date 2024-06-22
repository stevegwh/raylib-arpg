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
#include "ActorMovementSystem.hpp"
#include "NavigationGridSystem.hpp"

namespace sage
{

class ControllableActorSystem : public BaseSystem<ControllableActor>
{
    Cursor* cursor;
    UserInput* userInput;
    entt::entity controlledActorId{}; // TODO: Right now this system just controls one unit at a time. What if we wanted more?
    NavigationGridSystem* navigationGridSystem{};
    ActorMovementSystem* actorMovementSystem{};
    void onFloorClick(entt::entity entity);
    void onEnemyClick(entt::entity entity);
    void onTargetUpdate(entt::entity target);
    void cancelMovement(entt::entity entity);
public:
    ControllableActorSystem(entt::registry* _registry,
                                    Cursor* _cursor,
                                    UserInput* _userInput,
                                    NavigationGridSystem* _navigationGridSystem,
                                    ActorMovementSystem* _transformSystem);
    void Update() const;
    void MoveToLocation(entt::entity id);
    void PathfindToLocation(entt::entity id, Vector3 target);
    void SetControlledActor(entt::entity id);
    entt::entity GetControlledActor();

    void PatrolLocations(entt::entity id, const std::vector<Vector3> &patrol);
    
    entt::sigh<void(entt::entity)> onControlledActorChange;
    void Enable();
    void Disable();
};

} // sage
