//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "components/ControllableActor.hpp"

#include "BaseSystem.hpp"

#include "entt/entt.hpp"
#include "raylib.h"
#include "raymath.h"

#include <vector>

namespace sage
{
    class Cursor;               // Forward declaration
    class UserInput;            // Forward declaration
    class ActorMovementSystem;  // Forward declaration
    class NavigationGridSystem; // Forward declaration

    class ControllableActorSystem : public BaseSystem
    {
        Cursor* cursor;
        UserInput* userInput;
        entt::entity controlledActorId{};
        // TODO: Right now this system just controls one unit at a time. What if we wanted
        // more?
        NavigationGridSystem* navigationGridSystem{};
        ActorMovementSystem* actorMovementSystem{};
        void onFloorClick(entt::entity entity);
        void onEnemyClick(entt::entity clickedEntity);
        void onTargetUpdate(entt::entity target);

      public:
        ControllableActorSystem(
            entt::registry* _registry,
            Cursor* _cursor,
            UserInput* _userInput,
            NavigationGridSystem* _navigationGridSystem,
            ActorMovementSystem* _transformSystem);
        void Update() const;
        void MoveToLocation(entt::entity id);
        void PathfindToLocation(entt::entity id, Vector3 location);
        void CancelMovement(entt::entity entity);
        void SetControlledActor(entt::entity id);
        entt::entity GetControlledActor();

        void PatrolLocations(entt::entity id, const std::vector<Vector3>& patrol);
        bool ReachedDestination(entt::entity entity) const;

        entt::sigh<void(entt::entity)> onControlledActorChange;
        void Enable();
        void Disable();
    };
} // namespace sage
