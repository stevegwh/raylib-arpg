//
// Created by Steve Wheeler on 21/02/2024.
//
#pragma once
#include "BaseSystem.hpp"
#include "raylib.h"
#include <deque>
#include <vector>

namespace sage
{
    // Forward declarations
    class CollisionSystem;
    class NavigationGridSystem;
    struct MoveableActor;
    struct Collideable;
    struct sgTransform;
    struct GridSquare;
    struct NavigationGridSquare;

    class ActorMovementSystem : public BaseSystem
    {
      private:
        std::vector<Ray> debugRays;
        std::vector<RayCollision> debugCollisions;
        CollisionSystem* collisionSystem;
        NavigationGridSystem* navigationGridSystem;

        void clearDebugData();
        void processEntityWithCollision(
            entt::entity entity, MoveableActor& moveableActor, sgTransform& transform, Collideable& collideable);
        void processEntityWithoutCollision(
            entt::entity entity, MoveableActor& moveableActor, sgTransform& transform);
        bool isDestinationOccupied(const MoveableActor& moveableActor, const Collideable& collideable);
        void recalculatePath(entt::entity entity, MoveableActor& moveableActor, const Collideable& collideable);
        bool hasReachedNextPoint(const sgTransform& transform, const MoveableActor& moveableActor);
        void handlePointReached(
            entt::entity entity,
            sgTransform& transform,
            MoveableActor& moveableActor,
            const Collideable& collideable);
        void handlePointReachedWithoutCollision(
            entt::entity entity, sgTransform& transform, MoveableActor& moveableActor);
        void setPositionToGridCenter(
            entt::entity entity, sgTransform& transform, const MoveableActor& moveableActor);
        void handleDestinationReached(
            entt::entity entity, MoveableActor& moveableActor, const Collideable& collideable);
        void handleDestinationReachedWithoutCollision(entt::entity entity, MoveableActor& moveableActor);
        void handleActorCollisionAvoidance(
            entt::entity entity, const sgTransform& transform, MoveableActor& moveableActor);
        NavigationGridSquare* castCollisionRay(
            const GridSquare& actorIndex, const Vector3& direction, float distance, MoveableActor& moveableActor);
        void processOtherActorCollision(
            entt::entity entity,
            const sgTransform& transform,
            MoveableActor& moveableActor,
            NavigationGridSquare* hitCell);
        void updateActorPosition(entt::entity entity, sgTransform& transform, MoveableActor& moveableActor);
        void updateActorDirection(sgTransform& transform, const MoveableActor& moveableActor);
        void updateActorRotation(entt::entity entity, sgTransform& transform);
        void updateActorWorldPosition(entt::entity entity, sgTransform& transform);

      public:
        bool ReachedDestination(entt::entity entity) const;
        void PruneMoveCommands(const entt::entity& entity) const;
        // TODO: Overload this so you can just update one field at a time if needed
        void PathfindToLocation(const entt::entity& entity, const Vector3& destination);
        void MoveToLocation(const entt::entity& entity, Vector3 location);
        void CancelMovement(const entt::entity& entity) const;
        void Update() override;
        void DrawDebug() const;

        ActorMovementSystem(
            entt::registry* _registry,
            CollisionSystem* _collisionSystem,
            NavigationGridSystem* _navigationGridSystem);
    };

} // namespace sage