#include "ActorMovementSystem.hpp"
#include "CollisionSystem.hpp"
#include "components/MoveableActor.hpp"
#include "components/NavigationGridSquare.hpp"
#include "components/sgTransform.hpp"
#include "ControllableActorSystem.hpp"
#include "GameUiEngine.hpp"
#include "NavigationGridSystem.hpp"
#include "Serializer.hpp"
#include "slib.hpp"
#include "Systems.hpp"

#include <format>
#include <ranges>
#include <tuple>

namespace sage
{

    void ActorMovementSystem::PruneMoveCommands(const entt::entity& entity) const
    {
        auto& actor = registry->get<MoveableActor>(entity);
        std::deque<Vector3> empty;
        std::swap(actor.path, empty);
    }

    void ActorMovementSystem::CancelMovement(const entt::entity& entity) const
    {
        PruneMoveCommands(entity);
        auto& moveable = registry->get<MoveableActor>(entity);
        moveable.onMovementCancel.Publish(entity);
    }

    // TODO: If an object has a collideable, this is completely pointless, as it will inevitably use pathfinding if
    // it encounters any issues or collides with something. It would need to be updated differently, maybe the same
    // as the "non collideable" update. Moves to a location without pathfinding
    void ActorMovementSystem::MoveToLocation(const entt::entity& entity, Vector3 location) const
    {
        if (!registry->any_of<MoveableActor>(entity))
        {
            registry->emplace<MoveableActor>(entity);
        }
        PruneMoveCommands(entity);
        const auto& transform = registry->get<sgTransform>(entity);
        auto& moveableActor = registry->get<MoveableActor>(entity);
        moveableActor.path.emplace_back(transform.GetWorldPos());
        moveableActor.path.emplace_back(location);
        moveableActor.onStartMovement.Publish(entity);
    }

    bool ActorMovementSystem::TryPathfindToLocation(const entt::entity& entity, const Vector3& destination) const
    {
        PathfindToLocation(entity, destination);
        auto& moveable = registry->get<MoveableActor>(entity);
        return moveable.IsMoving();
    }

    void ActorMovementSystem::PathfindToLocation(const entt::entity& entity, const Vector3& destination) const
    {
        auto& moveable = registry->get<MoveableActor>(entity);

        if (!sys->navigationGridSystem->CheckWithinGridBounds(destination))
        {
            moveable.onDestinationUnreachable.Publish(entity, destination);
            if (entity == sys->controllableActorSystem->GetSelectedActor())
            {
                moveable.onDestinationUnreachable.Publish(entity, destination);
                sys->uiEngine->CreateErrorMessage("Out of bounds.");
            }
            // std::cout << std::format(
            // "Entity {}: Requested destination out of grid bounds \n", static_cast<int>(entity));

            return;
        }

        GridSquare minRange{};
        GridSquare maxRange{};
        if (!sys->navigationGridSystem->GetPathfindRange(entity, moveable.pathfindingBounds, minRange, maxRange))
        {
            // This will very rarely happen. Only triggers if the entity's current position is outside of grid
            // bounds.
            if (entity == sys->controllableActorSystem->GetSelectedActor())
            {
                sys->uiEngine->CreateErrorMessage("Out of bounds.");
            }
            // std::cout << std::format(
            // "Entity {}: Current position out of grid bounds \n", static_cast<int>(entity));
            moveable.onDestinationUnreachable.Publish(entity, destination);
            return;
        }

        if (!sys->navigationGridSystem->CheckWithinBounds(destination, minRange, maxRange))
        {
            if (entity == sys->controllableActorSystem->GetSelectedActor())
            {
                sys->uiEngine->CreateErrorMessage("Out of range.");
            }
            // std::cout << std::format(
            // "Entity {}: Requested destination is outside of pathfinding range \n", static_cast<int>(entity));
            moveable.onDestinationUnreachable.Publish(entity, destination);
            return;
        }

        const auto& collideable = registry->get<Collideable>(entity);
        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);

        const auto& actorTrans = registry->get<sgTransform>(entity);
        //        const auto path =
        //            navigationGridSystem->AStarPathfind(entity, actorTrans.GetWorldPos(), destination, minRange,
        //            maxRange);

        const auto path = sys->navigationGridSystem->BFSPathfind(
            entity, actorTrans.GetWorldPos(), destination, minRange, maxRange);

        if (moveable.IsMoving()) // Was previously moving
        {
            PruneMoveCommands(entity);
            moveable.onPathChanged.Publish(entity);
        }

        for (auto n : path)
        {
            moveable.path.emplace_back(n);
        }

        auto& transform = registry->get<sgTransform>(entity);

        if (!path.empty())
        {
            updateActorDirection(transform, moveable);
            updateActorRotation(entity, transform);
            moveable.onStartMovement.Publish(entity);
        }
        else
        {
            if (entity == sys->controllableActorSystem->GetSelectedActor())
            {
                sys->uiEngine->CreateErrorMessage("Destination unreachable.");
            }
            // std::cout << std::format(// "Entity {}: Destination unreachable \n", static_cast<int>(entity));
            moveable.onDestinationUnreachable.Publish(entity, destination);
        }

        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);
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
            for (auto p : actor.path)
            {
                DrawCube({p.x, p.y + 1, p.z}, 1, 1, 1, GREEN);
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

    bool ActorMovementSystem::isNextPointOccupied(
        const MoveableActor& moveableActor, const Collideable& collideable) const
    {
        return !sys->navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
            moveableActor.path.front(), collideable.worldBoundingBox);
    }

    void ActorMovementSystem::recalculatePath(
        const entt::entity entity, const MoveableActor& moveableActor, const Collideable& collideable) const
    {
        PathfindToLocation(entity, moveableActor.GetDestination());
    }

    bool ActorMovementSystem::hasReachedNextPoint(const sgTransform& transform, const MoveableActor& moveableActor)
    {
        // I do not believe that height should matter for this (could be very wrong)
        return Vector2Distance(
                   {moveableActor.path.front().x, moveableActor.path.front().z},
                   {transform.GetWorldPos().x, transform.GetWorldPos().z}) < 0.5f;
    }

    void ActorMovementSystem::handlePointReached(
        entt::entity entity, sgTransform& transform, MoveableActor& moveableActor) const
    {
        setPositionToGridCenter(transform, moveableActor);
        moveableActor.path.pop_front();

        if (moveableActor.path.empty())
        {
            handleDestinationReached(entity, moveableActor);
        }
    }

    void ActorMovementSystem::setPositionToGridCenter(
        sgTransform& transform, const MoveableActor& moveableActor) const
    {
        // Set continuous pos to grid/discrete pos
        GridSquare targetGridPos{};
        sys->navigationGridSystem->WorldToGridSpace(moveableActor.path.front(), targetGridPos);
        const auto square = sys->navigationGridSystem->GetGridSquare(targetGridPos.row, targetGridPos.col);
        transform.SetPosition({square->worldPosMin.x, square->GetTerrainHeight(), square->worldPosMin.z});
    }

    void ActorMovementSystem::handleDestinationReached(
        const entt::entity entity, const MoveableActor& moveableActor)
    {
        moveableActor.onDestinationReached.Publish(entity);
    }

    bool ActorMovementSystem::CheckCollisionWithOtherMoveable(
        const entt::entity entity, const sgTransform& transform, MoveableActor& moveableActor) const
    {
        constexpr float avoidanceDistance = 10;
        GridSquare actorIndex{};
        sys->navigationGridSystem->WorldToGridSpace(transform.GetWorldPos(), actorIndex);

        // navigationGridSystem->MarkSquaresDebug(moveableActor.debugRay, PURPLE, false);

        NavigationGridSquare* hitCell =
            castCollisionRay(actorIndex, transform.direction, avoidanceDistance, moveableActor);

        // If we haven't hit anything, or the object is static, then we don't need to worry about it.
        if (hitCell == nullptr || !registry->any_of<MoveableActor>(hitCell->occupant)) return false;

        const auto& hitTransform = registry->get<sgTransform>(hitCell->occupant);

        // Going same direction, ignore.
        auto dot = Vector3DotProduct(transform.direction, hitTransform.direction);
        if (dot >= 0)
        {
            return false;
        }

        if (registry->any_of<Collideable>(hitCell->occupant) &&
            (!moveableActor.followTarget.has_value() ||
             hitCell->occupant != moveableActor.followTarget->targetActor) &&
            moveableActor.hitEntityId != entity)
        {
            if (!AlmostEquals(hitTransform.GetWorldPos(), moveableActor.hitLastPos))
            {
                moveableActor.hitEntityId = hitCell->occupant;
                moveableActor.hitLastPos = hitTransform.GetWorldPos();

                auto& hitCol = registry->get<Collideable>(hitCell->occupant);

                if (Vector3Distance(hitTransform.GetWorldPos(), transform.GetWorldPos()) <
                    Vector3Distance(moveableActor.path.back(), transform.GetWorldPos()))
                {
                    // std::cout << std::format(
                    // "Entity {}: Collided with a moving object, rerouting \n", static_cast<int>(entity));
                    PathfindToLocation(entity, moveableActor.GetDestination());
                    hitCol.debugDraw = true;
                    return true;
                }
            }
        }
        return false;
    }

    NavigationGridSquare* ActorMovementSystem::castCollisionRay(
        const GridSquare& actorIndex, const Vector3& direction, float distance, MoveableActor& moveableActor) const
    {
        return sys->navigationGridSystem->CastRay(
            actorIndex.row, actorIndex.col, {direction.x, direction.z}, distance, moveableActor.debugRay);
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
        sys->navigationGridSystem->WorldToGridSpace(transform.GetWorldPos(), actorIndex);
        auto gridSquare = sys->navigationGridSystem->GetGridSquare(actorIndex.row, actorIndex.col);
        auto& moveable = registry->get<MoveableActor>(entity);
        Vector3 newPos = {
            transform.GetWorldPos().x + transform.direction.x * moveable.movementSpeed,
            gridSquare->GetTerrainHeight(),
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

    void ActorMovementSystem::updateActor(
        entt::entity entity, MoveableActor& moveableActor, sgTransform& transform, Collideable& collideable) const
    {
        if (moveableActor.path.empty())
        {
            return;
        }

        if (isNextPointOccupied(moveableActor, collideable))
        {
            // std::cout << std::format(// "Entity {}: Next point occupied, rerouting \n",
            // static_cast<int>(entity));
            recalculatePath(entity, moveableActor, collideable);
            return;
        }

        if (hasReachedNextPoint(transform, moveableActor))
        {
            handlePointReached(entity, transform, moveableActor);
            return;
        }

        if (!CheckCollisionWithOtherMoveable(entity, transform, moveableActor))
        {
            // TODO: Distance of the ray cast should be from the current pos to the next node
            updateActorTransform(entity, transform, moveableActor);
        }
    }

    void ActorMovementSystem::updateActor(
        const entt::entity entity, MoveableActor& moveableActor, sgTransform& transform) const
    {
        if (moveableActor.path.empty())
        {
            return;
        }

        if (hasReachedNextPoint(transform, moveableActor))
        {
            handlePointReached(entity, transform, moveableActor);
            return;
        }

        updateActorTransform(entity, transform, moveableActor);
    }

    void ActorMovementSystem::Update()
    {
        clearDebugData();

        auto fullView = registry->view<MoveableActor, sgTransform, Collideable>();
        for (auto [entity, moveableActor, transform, collideable] : fullView.each())
        {
            sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false);
            updateActor(entity, moveableActor, transform, collideable);
            sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true);
        }

        // Process entities without Collideable component (e.g., some abilities etc)
        auto partialView = registry->view<MoveableActor, sgTransform>(entt::exclude<Collideable>);
        for (auto [entity, moveableActor, transform] : partialView.each())
        {
            updateActor(entity, moveableActor, transform);
        }
    }

    ActorMovementSystem::ActorMovementSystem(entt::registry* _registry, Systems* _sys)
        : BaseSystem(_registry), sys(_sys)
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
//				//	// std::cout << "Hit with object: " << name << std::endl;
//				//}
//				//else
//				//{
//				//	auto text = TextFormat("Likely hit floor, with entity ID: %d",
// collisions.at(0).collidedEntityId);
//				//	// std::cout << text << std::endl;
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
//				// std::cout << "No getFirstCollision with terrain detected \n";
//			}