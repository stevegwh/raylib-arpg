#include "ActorMovementSystem.hpp"
#include "CollisionSystem.hpp"
#include "components/ControllableActor.hpp"
#include "components/MoveableActor.hpp"
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

    void ActorMovementSystem::MoveToLocation(const entt::entity& entity, Vector3 location)
    {
        if (!registry->any_of<MoveableActor>(entity))
        {
            registry->emplace<MoveableActor>(entity);
        }
        const auto& actorTrans = registry->get<sgTransform>(entity);
        PruneMoveCommands(entity);
        auto& transform = registry->get<sgTransform>(entity);
        auto& moveableActor = registry->get<MoveableActor>(entity);
        moveableActor.path.emplace_back(transform.GetWorldPos());
        moveableActor.path.emplace_back(location);
        transform.direction =
            Vector3Normalize(Vector3Subtract(moveableActor.path.front(), transform.GetWorldPos()));
        moveableActor.onStartMovement.publish(entity);
    }

    void ActorMovementSystem::PathfindToLocation(const entt::entity& entity, const Vector3& destination)
    // TODO: Pathfinding/movement needs some sense of movement speed.
    {
        if (!registry->any_of<MoveableActor>(entity))
        {
            registry->emplace<MoveableActor>(entity);
        }
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
        navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, false);
        GridSquare minRange{};
        GridSquare maxRange{};
        navigationGridSystem->GetPathfindRange(entity, bounds, minRange, maxRange);
        {
            // If location outside of actor's movement range, then return
            GridSquare tmp{};
            if (!navigationGridSystem->WorldToGridSpace(destination, tmp, minRange, maxRange)) return;
        }
        navigationGridSystem->DrawDebugPathfinding(minRange, maxRange);

        const auto& actorTrans = registry->get<sgTransform>(entity);
        // auto path = navigationGridSystem->BFSPathfind(entity, actorTrans.position(),
        // destination, minRange, maxRange);
        auto path =
            navigationGridSystem->AStarPathfind(entity, actorTrans.GetWorldPos(), destination, minRange, maxRange);
        PruneMoveCommands(entity);
        auto& transform = registry->get<sgTransform>(entity);
        auto& moveableActor = registry->get<MoveableActor>(entity);
        for (auto n : path)
            moveableActor.path.emplace_back(n);
        if (!path.empty())
        {
            updateActorDirection(transform, moveableActor);
            updateActorRotation(entity, transform);
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
            DrawLine3D(ray.position, Vector3Add(ray.position, Vector3Multiply(ray.direction, {5, 1, 5})), RED);
        }

        for (auto& col : debugCollisions)
        {
            DrawSphere(col.point, 1.5, GREEN);
        }
    }

    void ActorMovementSystem::Update()
    {
        // clearDebugData();

        // Process entities with all three components
        auto fullView = registry->view<MoveableActor, sgTransform, Collideable>();
        for (auto [entity, moveableActor, transform, collideable] : fullView.each())
        {
            processEntityWithCollision(entity, moveableActor, transform, collideable);
        }

        // Process entities without Collideable component (e.g., some abilities etc)
        auto partialView = registry->view<MoveableActor, sgTransform>(entt::exclude<Collideable>);
        for (auto [entity, moveableActor, transform] : partialView.each())
        {
            processEntityWithoutCollision(entity, moveableActor, transform);
        }
    }

    void ActorMovementSystem::clearDebugData()
    {
        debugRays.erase(debugRays.begin(), debugRays.end());
        debugCollisions.erase(debugCollisions.begin(), debugCollisions.end());
    }

    void ActorMovementSystem::processEntityWithCollision(
        entt::entity entity, MoveableActor& moveableActor, sgTransform& transform, Collideable& collideable)
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
        updateActorPosition(entity, transform, moveableActor);
        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
    }

    void ActorMovementSystem::processEntityWithoutCollision(
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

        updateActorPosition(entity, transform, moveableActor);
    }

    bool ActorMovementSystem::isDestinationOccupied(
        const MoveableActor& moveableActor, const Collideable& collideable)
    {
        // TODO: Works when I check if "back" is occupied, but not when I check if "front" is occupied. Why?
        // Checks whether the next destination is occupied, and if it is, then recalculate the path.
        return !navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
                   moveableActor.path.front(), collideable.worldBoundingBox) ||
               !navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
                   moveableActor.path.back(), collideable.worldBoundingBox);
    }

    void ActorMovementSystem::recalculatePath(
        entt::entity entity, MoveableActor& moveableActor, const Collideable& collideable)
    {
        PathfindToLocation(entity, moveableActor.path.back());
        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
    }

    bool ActorMovementSystem::hasReachedNextPoint(const sgTransform& transform, const MoveableActor& moveableActor)
    {
        return Vector3Distance(moveableActor.path.front(), transform.GetWorldPos()) < 0.5f; // Destination reached
    }

    void ActorMovementSystem::handlePointReached(
        entt::entity entity, sgTransform& transform, MoveableActor& moveableActor, const Collideable& collideable)
    {
        setPositionToGridCenter(entity, transform, moveableActor);
        moveableActor.path.pop_front();

        if (moveableActor.path.empty())
        {
            handleDestinationReached(entity, moveableActor, collideable);
        }
    }

    void ActorMovementSystem::handlePointReachedWithoutCollision(
        entt::entity entity, sgTransform& transform, MoveableActor& moveableActor)
    {
        setPositionToGridCenter(entity, transform, moveableActor);
        moveableActor.path.pop_front();

        if (moveableActor.path.empty())
        {
            handleDestinationReachedWithoutCollision(entity, moveableActor);
        }
    }

    void ActorMovementSystem::setPositionToGridCenter(
        entt::entity entity, sgTransform& transform, const MoveableActor& moveableActor)
    {
        // Set continuous pos to grid/discrete pos
        GridSquare targetGridPos{};
        navigationGridSystem->WorldToGridSpace(moveableActor.path.front(), targetGridPos);
        auto square = navigationGridSystem->GetGridSquare(targetGridPos.row, targetGridPos.col);
        transform.SetPosition({square->worldPosMin.x, square->terrainHeight, square->worldPosMin.z});
    }

    void ActorMovementSystem::handleDestinationReached(
        entt::entity entity, MoveableActor& moveableActor, const Collideable& collideable)
    {
        moveableActor.onDestinationReached.publish(entity);
        moveableActor.onFinishMovement.publish(entity);
        navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
    }

    void ActorMovementSystem::handleDestinationReachedWithoutCollision(
        entt::entity entity, MoveableActor& moveableActor)
    {
        moveableActor.onDestinationReached.publish(entity);
        moveableActor.onFinishMovement.publish(entity);
    }

    void ActorMovementSystem::handleActorCollisionAvoidance(
        entt::entity entity, const sgTransform& transform, MoveableActor& moveableActor)
    {
        float avoidanceDistance = 10;
        GridSquare actorIndex{};
        navigationGridSystem->WorldToGridSpace(transform.GetWorldPos(), actorIndex);

        navigationGridSystem->MarkSquaresDebug(moveableActor.debugRay, PURPLE, false);
        // Looks ahead and checks if getFirstCollision will occur
        NavigationGridSquare* hitCell =
            castCollisionRay(actorIndex, transform.direction, avoidanceDistance, moveableActor);

        if (hitCell != nullptr && registry->any_of<Collideable>(hitCell->occupant))
        {
            processOtherActorCollision(entity, transform, moveableActor, hitCell);
        }
    }

    NavigationGridSquare* ActorMovementSystem::castCollisionRay(
        const GridSquare& actorIndex, const Vector3& direction, float distance, MoveableActor& moveableActor)
    {
        return navigationGridSystem->CastRay(
            actorIndex.row, actorIndex.col, {direction.x, direction.z}, distance, moveableActor.debugRay);
    }

    void ActorMovementSystem::processOtherActorCollision(
        entt::entity entity,
        const sgTransform& transform,
        MoveableActor& moveableActor,
        NavigationGridSquare* hitCell)
    {
        auto& hitTransform = registry->get<sgTransform>(hitCell->occupant);
        if (moveableActor.lastHitActor != entity ||
            !AlmostEquals(hitTransform.GetWorldPos(), moveableActor.hitActorLastPos))
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

    void ActorMovementSystem::updateActorPosition(
        entt::entity entity, sgTransform& transform, MoveableActor& moveableActor)
    {
        updateActorDirection(transform, moveableActor);
        updateActorRotation(entity, transform);
        updateActorWorldPosition(entity, transform);
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

    void ActorMovementSystem::updateActorWorldPosition(entt::entity entity, sgTransform& transform)
    {
        GridSquare actorIndex{};
        navigationGridSystem->WorldToGridSpace(transform.GetWorldPos(), actorIndex);
        auto gridSquare = navigationGridSystem->GetGridSquare(actorIndex.row, actorIndex.col);

        Vector3 newPos = {
            transform.GetWorldPos().x + transform.direction.x * transform.movementSpeed,
            gridSquare->terrainHeight,
            transform.GetWorldPos().z + transform.direction.z * transform.movementSpeed};

        transform.SetPosition(newPos);
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