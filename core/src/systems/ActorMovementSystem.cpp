#include "ActorMovementSystem.hpp"
#include "CollisionSystem.hpp"
#include "components/ControllableActor.hpp"
#include "components/MoveableActor.hpp"
#include "components/NavigationGridSquare.hpp"
#include "components/sgTransform.hpp"
#include "NavigationGridSystem.hpp"
#include <Serializer.hpp>

#include <ranges>
#include <tuple>

bool AlmostEquals(Vector3 a, Vector3 b)
{
    const std::tuple aInt = {static_cast<int>(a.x), static_cast<int>(a.z)};
    const std::tuple bInt = {static_cast<int>(b.x), static_cast<int>(b.z)};
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
        auto& moveable = registry->get<MoveableActor>(entity);
        moveable.targetActor = entt::null;
        moveable.lastHitActor = entt::null;
        moveable.onMovementCancel.publish(entity);
    }

    // Moves to a location without pathfinding
    void ActorMovementSystem::MoveToLocation(const entt::entity& entity, Vector3 location) const
    {
        if (!registry->any_of<MoveableActor>(entity))
        {
            registry->emplace<MoveableActor>(entity);
        }
        PruneMoveCommands(entity);
        auto& transform = registry->get<sgTransform>(entity);
        auto& moveableActor = registry->get<MoveableActor>(entity);
        moveableActor.path.emplace_back(transform.GetWorldPos());
        moveableActor.path.emplace_back(location);
        moveableActor.onStartMovement.publish(entity);
    }

    void ActorMovementSystem::PathfindToLocation(const entt::entity& entity, const Vector3& destination) const
    {
        if (!registry->any_of<MoveableActor>(entity))
        {
            registry->emplace<MoveableActor>(entity);
        }

        if (!navigationGridSystem->CheckWithinGridBounds(destination))
        {
            std::cout << "ActorMovementSystem: Requested destination out of grid bounds. \n";
            return;
        }

        const auto& collideable = registry->get<Collideable>(entity);
        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);

        auto& moveable = registry->get<MoveableActor>(entity);
        GridSquare minRange{};
        GridSquare maxRange{};
        if (!navigationGridSystem->GetPathfindRange(entity, moveable.pathfindingBounds, minRange, maxRange))
        {
            std::cout << "ActorMovementSystem: Requested destination out of grid bounds. \n";
            return;
        }
        const auto& actorTrans = registry->get<sgTransform>(entity);
        auto path =
            navigationGridSystem->AStarPathfind(entity, actorTrans.GetWorldPos(), destination, minRange, maxRange);
        PruneMoveCommands(entity);
        auto& transform = registry->get<sgTransform>(entity);

        for (auto n : path)
        {
            moveable.path.emplace_back(n);
        }

        if (!path.empty())
        {
            updateActorDirection(transform, moveable);
            updateActorRotation(entity, transform);
            moveable.onStartMovement.publish(entity);
        }
        else
        {
            std::cout << "ActorMovementSystem: Destination unreachable \n";
            CancelMovement(entity);
        }
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
            DrawLine3D(ray.position, Vector3Add(ray.position, Vector3Multiply(ray.direction, {5, 1, 5})), RED);
        }

        for (auto& col : debugCollisions)
        {
            DrawSphere(col.point, 1.5, GREEN);
        }
    }

    void ActorMovementSystem::clearDebugData()
    {
        debugRays.erase(debugRays.begin(), debugRays.end());
        debugCollisions.erase(debugCollisions.begin(), debugCollisions.end());
    }

    bool ActorMovementSystem::isDestinationOccupied(
        const MoveableActor& moveableActor, const Collideable& collideable) const
    {
        // TODO: Works when I check if "back" is occupied, but not when I check if "front" is occupied. Why?
        // Checks whether the next destination is occupied, and if it is, then recalculate the path.
        return !navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
                   moveableActor.path.front(), collideable.worldBoundingBox) ||
               !navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
                   moveableActor.path.back(), collideable.worldBoundingBox);
    }

    void ActorMovementSystem::recalculatePath(
        const entt::entity entity, const MoveableActor& moveableActor, const Collideable& collideable) const
    {
        PathfindToLocation(entity, moveableActor.path.back());
        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
    }

    bool ActorMovementSystem::hasReachedNextPoint(const sgTransform& transform, const MoveableActor& moveableActor)
    {
        return Vector3Distance(moveableActor.path.front(), transform.GetWorldPos()) < 0.5f; // Destination reached
    }

    void ActorMovementSystem::handlePointReached(
        entt::entity entity,
        sgTransform& transform,
        MoveableActor& moveableActor,
        const Collideable& collideable) const
    {
        setPositionToGridCenter(entity, transform, moveableActor);
        moveableActor.path.pop_front();

        if (moveableActor.path.empty())
        {
            handleDestinationReached(entity, moveableActor, collideable);
        }
    }

    void ActorMovementSystem::handlePointReachedWithoutCollision(
        entt::entity entity, sgTransform& transform, MoveableActor& moveableActor) const
    {
        setPositionToGridCenter(entity, transform, moveableActor);
        moveableActor.path.pop_front();

        if (moveableActor.path.empty())
        {
            handleDestinationReachedWithoutCollision(entity, moveableActor);
        }
    }

    void ActorMovementSystem::setPositionToGridCenter(
        entt::entity entity, sgTransform& transform, const MoveableActor& moveableActor) const
    {
        // Set continuous pos to grid/discrete pos
        GridSquare targetGridPos{};
        navigationGridSystem->WorldToGridSpace(moveableActor.path.front(), targetGridPos);
        const auto square = navigationGridSystem->GetGridSquare(targetGridPos.row, targetGridPos.col);
        transform.SetPosition({square->worldPosMin.x, square->terrainHeight, square->worldPosMin.z});
    }

    void ActorMovementSystem::handleDestinationReached(
        const entt::entity entity, const MoveableActor& moveableActor, const Collideable& collideable) const
    {
        moveableActor.onDestinationReached.publish(entity);
        moveableActor.onFinishMovement.publish(entity);
        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
    }

    void ActorMovementSystem::handleDestinationReachedWithoutCollision(
        const entt::entity entity, const MoveableActor& moveableActor)
    {
        moveableActor.onDestinationReached.publish(entity);
        moveableActor.onFinishMovement.publish(entity);
    }

    void ActorMovementSystem::handleActorCollisionAvoidance(
        const entt::entity entity, const sgTransform& transform, MoveableActor& moveableActor) const
    {
        float avoidanceDistance = 10;
        GridSquare actorIndex{};
        navigationGridSystem->WorldToGridSpace(transform.GetWorldPos(), actorIndex);

        // navigationGridSystem->MarkSquaresDebug(moveableActor.debugRay, PURPLE, false);
        // Looks ahead and checks if getFirstCollision will occur
        NavigationGridSquare* hitCell =
            castCollisionRay(actorIndex, transform.direction, avoidanceDistance, moveableActor);

        if (hitCell != nullptr && registry->any_of<Collideable>(hitCell->occupant))
        {
            processOtherActorCollision(entity, transform, moveableActor, hitCell);
        }
    }

    NavigationGridSquare* ActorMovementSystem::castCollisionRay(
        const GridSquare& actorIndex, const Vector3& direction, float distance, MoveableActor& moveableActor) const
    {
        return navigationGridSystem->CastRay(
            actorIndex.row, actorIndex.col, {direction.x, direction.z}, distance, moveableActor.debugRay);
    }

    void ActorMovementSystem::processOtherActorCollision(
        const entt::entity& entity,
        const sgTransform& transform,
        MoveableActor& moveableActor,
        const NavigationGridSquare* hitCell) const
    {
        // If we're supposed to be moving towards this actor, then ignore collision with it.
        if (hitCell->occupant == moveableActor.targetActor || moveableActor.lastHitActor == entity) return;

        auto& hitTransform = registry->get<sgTransform>(hitCell->occupant);
        if (!AlmostEquals(hitTransform.GetWorldPos(), moveableActor.hitActorLastPos))
        {
            moveableActor.lastHitActor = entity;
            moveableActor.hitActorLastPos = hitTransform.GetWorldPos();

            auto& hitCol = registry->get<Collideable>(hitCell->occupant);

            if (Vector3Distance(hitTransform.GetWorldPos(), transform.GetWorldPos()) <
                Vector3Distance(moveableActor.path.back(), transform.GetWorldPos()))
            {
                PathfindToLocation(entity, moveableActor.path.back());
                hitCol.debugDraw = true;
            }
        }
    }

    void ActorMovementSystem::updateActorDirection(sgTransform& transform, const MoveableActor& moveableActor)
    {
        if (moveableActor.path.empty()) return;
        transform.direction =
            Vector3Normalize(Vector3Subtract(moveableActor.path.front(), transform.GetWorldPos()));
    }

    void ActorMovementSystem::updateActorRotation(entt::entity entity, sgTransform& transform)
    {
        // Calculate rotation angle based on direction
        float angle = atan2f(transform.direction.x, transform.direction.z) * RAD2DEG;
        transform.SetRotation({transform.GetWorldRot().x, angle, transform.GetWorldRot().z});
    }

    void ActorMovementSystem::updateActorWorldPosition(entt::entity entity, sgTransform& transform) const
    {
        GridSquare actorIndex{};
        navigationGridSystem->WorldToGridSpace(transform.GetWorldPos(), actorIndex);
        auto gridSquare = navigationGridSystem->GetGridSquare(actorIndex.row, actorIndex.col);
        auto& moveable = registry->get<MoveableActor>(entity);
        Vector3 newPos = {
            transform.GetWorldPos().x + transform.direction.x * moveable.movementSpeed,
            gridSquare->terrainHeight,
            transform.GetWorldPos().z + transform.direction.z * moveable.movementSpeed};

        transform.SetPosition(newPos);
    }

    void ActorMovementSystem::updateActorTransform(
        entt::entity entity, sgTransform& transform, MoveableActor& moveableActor) const
    {
        updateActorDirection(transform, moveableActor);
        updateActorRotation(entity, transform);
        updateActorWorldPosition(entity, transform);
    }

    void ActorMovementSystem::updateActorCollideable(
        entt::entity entity, MoveableActor& moveableActor, sgTransform& transform, Collideable& collideable) const
    {
        if (moveableActor.path.empty())
        {
            return;
        }

        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);

        if (isDestinationOccupied(moveableActor, collideable))
        {
            recalculatePath(entity, moveableActor, collideable);
            return;
        }

        if (hasReachedNextPoint(transform, moveableActor))
        {
            handlePointReached(entity, transform, moveableActor, collideable);
            return;
        }

        handleActorCollisionAvoidance(entity, transform, moveableActor);
        updateActorTransform(entity, transform, moveableActor);
        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
    }

    void ActorMovementSystem::updateActorNonCollideable(
        entt::entity entity, MoveableActor& moveableActor, sgTransform& transform)
    {
        if (moveableActor.path.empty())
        {
            return;
        }

        if (hasReachedNextPoint(transform, moveableActor))
        {
            handlePointReachedWithoutCollision(entity, transform, moveableActor);
            return;
        }

        updateActorTransform(entity, transform, moveableActor);
    }

    void ActorMovementSystem::Update()
    {
        // clearDebugData();

        // Process entities with all three components
        auto fullView = registry->view<MoveableActor, sgTransform, Collideable>();
        for (auto [entity, moveableActor, transform, collideable] : fullView.each())
        {
            updateActorCollideable(entity, moveableActor, transform, collideable);
        }

        // Process entities without Collideable component (e.g., some abilities etc)
        auto partialView = registry->view<MoveableActor, sgTransform>(entt::exclude<Collideable>);
        for (auto [entity, moveableActor, transform] : partialView.each())
        {
            updateActorNonCollideable(entity, moveableActor, transform);
        }
    }

    ActorMovementSystem::ActorMovementSystem(
        entt::registry* _registry, CollisionSystem* _collisionSystem, NavigationGridSystem* _navigationGridSystem)
        : BaseSystem(_registry), collisionSystem(_collisionSystem), navigationGridSystem(_navigationGridSystem)
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
//				std::cout << "No getFirstCollision with terrain detected \n";
//			}