//
// Created by Steve Wheeler on 21/02/2024.
//

#include "ActorMovementSystem.hpp"

#include "CollisionSystem.hpp"
#include "components/ControllableActor.hpp"
#include "components/MovableActor.hpp"
#include "components/NavigationGridSquare.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "NavigationGridSystem.hpp"
#include <Serializer.hpp>
#include <slib.hpp>

#include <ranges>
#include <tuple>

bool AlmostEquals(Vector3 a, Vector3 b)
{
    std::tuple<int, int> aInt = {static_cast<int>(a.x), static_cast<int>(a.z)};
    std::tuple<int, int> bInt = {static_cast<int>(b.x), static_cast<int>(b.z)};
    return aInt == bInt;
}

namespace sage
{
    void ActorMovementSystem::PruneMoveCommands(const entt::entity& entity) const
    {
        auto& actor = registry->get<MoveableActor>(entity);
        {
            std::deque<Vector3> empty;
            std::swap(actor.path, empty);
        }
    }

    void ActorMovementSystem::CancelMovement(const entt::entity& entity) const
    {
        PruneMoveCommands(entity);
        auto& moveableActor = registry->get<MoveableActor>(entity);
        moveableActor.onMovementCancel.publish(entity);
    }

    void ActorMovementSystem::PathfindToLocation(
        const entt::entity& entity, const Vector3& destination)
    // TODO: Pathfinding/movement needs some sense of movement speed.
    {
        {
            // If location outside of bounds, then return
            GridSquare tmp{};
            if (!navigationGridSystem->WorldToGridSpace(destination, tmp)) return;
        }

        int bounds = 50;
        if (registry->any_of<ControllableActor>(entity))
        // TODO: Why is this only for controllable actors? Shouldn't all actors have
        // pathfinding bounds?
        {
            const auto& controllableActor = registry->get<ControllableActor>(entity);
            bounds = controllableActor.pathfindingBounds;
        }

        const auto& actorCollideable = registry->get<Collideable>(entity);
        navigationGridSystem->MarkSquareAreaOccupied(
            actorCollideable.worldBoundingBox, false);
        GridSquare minRange{};
        GridSquare maxRange{};
        navigationGridSystem->GetPathfindRange(entity, bounds, minRange, maxRange);
        {
            // If location outside of actor's movement range, then return
            GridSquare tmp{};
            if (!navigationGridSystem->WorldToGridSpace(
                    destination, tmp, minRange, maxRange))
                return;
        }
        navigationGridSystem->DrawDebugPathfinding(minRange, maxRange);

        const auto& actorTrans = registry->get<sgTransform>(entity);
        // auto path = navigationGridSystem->BFSPathfind(entity, actorTrans.position(),
        // destination, minRange, maxRange);
        auto path = navigationGridSystem->AStarPathfind(
            entity, actorTrans.position(), destination, minRange, maxRange);
        PruneMoveCommands(entity);
        auto& transform = registry->get<sgTransform>(entity);
        auto& moveableActor = registry->get<MoveableActor>(entity);
        for (auto n : path)
            moveableActor.path.emplace_back(n);
        if (!path.empty())
        {
            transform.direction = Vector3Normalize(
                Vector3Subtract(moveableActor.path.front(), transform.position()));
            moveableActor.onStartMovement.publish(entity);
        }
        // TODO: Handle destination being unreachable. (Change animation to IDLE, for a
        // start)
    }

    bool ActorMovementSystem::ReachedDestination(entt::entity entity) const
    {
        const auto& actor = registry->get<MoveableActor>(entity);
        return actor.path.empty();
    }

    void ActorMovementSystem::DrawDebug() const
    {
        auto view = registry->view<MoveableActor, sgTransform>();
        for (auto& entity : view)
        {
            auto& actor = registry->get<MoveableActor>(entity);
            if (actor.path.empty()) continue;
            auto& transform = registry->get<sgTransform>(entity);
            if (!actor.path.empty())
            {
                for (auto p : actor.path)
                {
                    DrawCube({p.x, p.y + 1, p.z}, 1, 1, 1, GREEN);
                }
            }
        }

        for (auto& ray : debugRays)
        {
            DrawLine3D(
                ray.position,
                Vector3Add(ray.position, Vector3Multiply(ray.direction, {5, 1, 5})),
                RED);
        }

        for (auto& col : debugCollisions)
        {
            DrawSphere(col.point, 1.5, GREEN);
        }
    }

    void ActorMovementSystem::Update()
    {
        // TODO: Instead of always calling this no matter what... maybe choose to call it
        // (or not) based on the state. So, you can pass in the entity to Update and
        // iterate over a collectioon from another system
        debugRays.erase(debugRays.begin(), debugRays.end());
        debugCollisions.erase(debugCollisions.begin(), debugCollisions.end());
        auto view = registry->view<MoveableActor, sgTransform>();
        for (auto& entity : view)
        {
            auto& actorTrans = registry->get<sgTransform>(entity);
            auto& moveableActor = registry->get<MoveableActor>(entity);
            const auto& actorCollideable = registry->get<Collideable>(entity);

            if (moveableActor.path.empty())
            {
                continue;
            }

            navigationGridSystem->MarkSquareAreaOccupied(
                actorCollideable.worldBoundingBox, false);
            auto nextPointDist =
                Vector3Distance(moveableActor.path.front(), actorTrans.position());

            //			// TODO: Works when I check if "back" is occupied, but not when I
            // check if "front" is occupied. Why?
            //			// Checks whether the next destination is occupied, and if it is,
            // then recalculate the path.
            if (!navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
                    moveableActor.path.front(), actorCollideable.worldBoundingBox) ||
                !navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
                    moveableActor.path.back(), actorCollideable.worldBoundingBox))
            {
                PathfindToLocation(entity, moveableActor.path.back());
                navigationGridSystem->MarkSquareAreaOccupied(
                    actorCollideable.worldBoundingBox, true, entity);
                continue;
            }

            if (nextPointDist < 0.5f) // Destination reached
            {
                // Set continuous pos to grid/discrete pos
                GridSquare targetGridPos{};
                navigationGridSystem->WorldToGridSpace(
                    moveableActor.path.front(), targetGridPos);
                auto square = navigationGridSystem->GetGridSquare(
                    targetGridPos.row, targetGridPos.col);
                actorTrans.SetPosition(
                    {square->worldPosMin.x, square->terrainHeight, square->worldPosMin.z},
                    entity);

                moveableActor.path.pop_front();
                if (moveableActor.path.empty())
                {
                    moveableActor.onDestinationReached.publish(entity);
                    moveableActor.onFinishMovement.publish(entity);
                    navigationGridSystem->MarkSquareAreaOccupied(
                        actorCollideable.worldBoundingBox, true, entity);
                    continue;
                }
            }

            float avoidanceDistance = 10;
            GridSquare actorIndex{};
            navigationGridSystem->WorldToGridSpace(actorTrans.position(), actorIndex);

            navigationGridSystem->MarkSquaresDebug(moveableActor.debugRay, PURPLE, false);
            // Looks ahead and checks if collision will occur
            NavigationGridSquare* hitCell = navigationGridSystem->CastRay(
                actorIndex.row,
                actorIndex.col,
                {actorTrans.direction.x, actorTrans.direction.z},
                avoidanceDistance,
                moveableActor.debugRay);

            if (hitCell != nullptr)
            {
                auto& hitTransform = registry->get<sgTransform>(hitCell->occupant);
                if (moveableActor.lastHitActor != entity ||
                    !AlmostEquals(hitTransform.position(), moveableActor.hitActorLastPos))
                {
                    moveableActor.lastHitActor = entity;
                    moveableActor.hitActorLastPos = hitTransform.position();

                    auto& hitCol = registry->get<Collideable>(hitCell->occupant);

                    if (Vector3Distance(hitTransform.position(), actorTrans.position()) <
                        Vector3Distance(moveableActor.path.back(), actorTrans.position()))
                    {
                        PathfindToLocation(entity, moveableActor.path.back());
                        hitCol.debugDraw = true;
                    }
                }
            }

            actorTrans.direction = Vector3Normalize(
                Vector3Subtract(moveableActor.path.front(), actorTrans.position()));
            // Calculate rotation angle based on direction
            float angle =
                atan2f(actorTrans.direction.x, actorTrans.direction.z) * RAD2DEG;
            actorTrans.SetRotation(
                {actorTrans.rotation().x, angle, actorTrans.rotation().z}, entity);

            auto gridSquare =
                navigationGridSystem->GetGridSquare(actorIndex.row, actorIndex.col);
            Vector3 newPos = {
                actorTrans.position().x +
                    actorTrans.direction.x * actorTrans.movementSpeed,
                gridSquare->terrainHeight,
                actorTrans.position().z +
                    actorTrans.direction.z * actorTrans.movementSpeed};

            actorTrans.SetPosition(newPos, entity);

            navigationGridSystem->MarkSquareAreaOccupied(
                actorCollideable.worldBoundingBox, true, entity);
        }
    }

    ActorMovementSystem::ActorMovementSystem(
        entt::registry* _registry,
        CollisionSystem* _collisionSystem,
        NavigationGridSystem* _navigationGridSystem)
        : BaseSystem(_registry),
          collisionSystem(_collisionSystem),
          navigationGridSystem(_navigationGridSystem)
    {
    }
} // namespace sage

// Below: Old terrain height calculation before height maps
//			float newY = 0;
//			Ray ray;
//			ray.position = actorTrans.position();
//			ray.position.y = actorCollideable.worldBoundingBox.max.y;
//			ray.direction = { 0, -1, 0 };
//			debugRays.push_back(ray);
//			auto collisions = collisionSystem->GetMeshCollisionsWithRay(entity, ray,
// CollisionLayer::NAVIGATION); 			if (!collisions.empty())
//			{
//				//auto hitentt = collisions.at(0).collidedEntityId;
//				//if (registry->any_of<Renderable>(hitentt))
//				//{
//				//	auto& name =
// registry->get<Renderable>(collisions.at(0).collidedEntityId).name;
//				//	std::cout << "Hit with object: " << name << std::endl;
//				//}
//				//else
//				//{
//				//	auto text = TextFormat("Likely hit floor, with entity ID: %d",
// collisions.at(0).collidedEntityId);
//				//	std::cout << text << std::endl;
//				//}
//
//
//				//auto newPos = Vector3Subtract(actorCollideable.localBoundingBox.max,
// collisions.at(0).rlCollision.point); 				newY =
// collisions.at(0).rlCollision.point.y;
//				//debugCollisions.push_back(collisions.at(0).rlCollision);
//
//			}
//			else
//			{
//				std::cout << "No collision with terrain detected \n";
//			}