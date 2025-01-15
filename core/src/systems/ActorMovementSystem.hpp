//
// Created by Steve Wheeler on 21/02/2024.
//
#pragma once

#include "BaseSystem.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    // Forward declarations
    struct Systems;
    struct MoveableActor;
    class Collideable;
    struct sgTransform;
    struct GridSquare;
    struct NavigationGridSquare;

    class ActorMovementSystem : public BaseSystem
    {
        Systems* sys;
        std::vector<Ray> debugRays;
        std::vector<RayCollision> debugCollisions;

        void clearDebugData();
        void updateActor(
            entt::entity entity,
            MoveableActor& moveableActor,
            sgTransform& transform,
            Collideable& collideable) const;
        void updateActor(entt::entity entity, MoveableActor& moveableActor, sgTransform& transform) const;
        [[nodiscard]] bool isNextPointOccupied(
            const MoveableActor& moveableActor, const Collideable& collideable) const;
        void recalculatePath(
            entt::entity entity, const MoveableActor& moveableActor, const Collideable& collideable) const;
        static bool hasReachedNextPoint(const sgTransform& transform, const MoveableActor& moveableActor);
        void handlePointReached(entt::entity entity, sgTransform& transform, MoveableActor& moveableActor) const;
        void setPositionToGridCenter(sgTransform& transform, const MoveableActor& moveableActor) const;
        static void handleDestinationReached(entt::entity entity, const MoveableActor& moveableActor);
        NavigationGridSquare* castCollisionRay(
            const GridSquare& actorIndex,
            const Vector3& direction,
            float distance,
            MoveableActor& moveableActor) const;
        void updateActorTransform(entt::entity entity, sgTransform& transform, MoveableActor& moveableActor) const;
        static void updateActorDirection(sgTransform& transform, const MoveableActor& moveableActor);
        static void updateActorRotation(entt::entity entity, sgTransform& transform);
        void updateActorWorldPosition(entt::entity entity, sgTransform& transform) const;

      public:
        [[nodiscard]] bool CheckCollisionWithOtherMoveable(
            entt::entity entity, const sgTransform& transform, MoveableActor& moveableActor) const;
        [[nodiscard]] bool ReachedDestination(entt::entity entity) const;
        void PruneMoveCommands(const entt::entity& entity) const;
        [[nodiscard]] bool TryPathfindToLocation(const entt::entity& entity, const Vector3& destination) const;
        void PathfindToLocation(const entt::entity& entity, const Vector3& destination) const;
        void MoveToLocation(const entt::entity& entity, Vector3 location) const;
        void CancelMovement(const entt::entity& entity) const;
        void Update() override;
        void DrawDebug() const;

        ActorMovementSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage