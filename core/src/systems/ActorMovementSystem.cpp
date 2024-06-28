//
// Created by Steve Wheeler on 21/02/2024.
//

#include "ActorMovementSystem.hpp"

#include "../components/NavigationGridSquare.hpp"

#include "../../utils/Serializer.hpp"

#include <tuple>
#include <ranges>

#include "components/ControllableActor.hpp"

namespace sage
{


void ActorMovementSystem::PruneMoveCommands(const entt::entity& entity) const
{
    auto& actor = registry->get<MoveableActor>(entity);
    {
        std::deque<Vector3> empty;
        std::swap(actor.globalPath, empty);
    }

}

void ActorMovementSystem::CancelMovement(const entt::entity& entity) const
{
    PruneMoveCommands(entity);
    auto& transform = registry->get<Transform>(entity);
    transform.onMovementCancel.publish(entity);
}


void ActorMovementSystem::PathfindToLocation(const entt::entity& entity, const Vector3& destination, bool initialMove) // TODO: Pathfinding/movement needs some sense of movement speed.
{

	{
        // If location outside of bounds, then return
        GridSquare tmp;
        if (!navigationGridSystem->WorldToGridSpace(destination, tmp)) return;
    }

    int bounds = 50;
	if (registry->any_of<ControllableActor>(entity)) // TODO: Why is this only for controllable actors? Shouldn't all actors have pathfinding bounds?
	{
		const auto& controllableActor = registry->get<ControllableActor>(entity);
		bounds = controllableActor.pathfindingBounds;
	}

    const auto& actorCollideable = registry->get<Collideable>(entity);
    navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, false);
    GridSquare minRange;
    GridSquare maxRange;
    navigationGridSystem->GetPathfindRange(entity, bounds, minRange, maxRange);
    {
        // If location outside of actor's movement range, then return
        GridSquare tmp;
        if (!navigationGridSystem->WorldToGridSpace(destination, tmp, minRange, maxRange)) return;
    }
    navigationGridSystem->DrawDebugPathfinding(minRange, maxRange);


    const auto& actorTrans = registry->get<Transform>(entity);
    auto path = navigationGridSystem->AStarPathfind(entity, actorTrans.position, destination, minRange, maxRange);

    PruneMoveCommands(entity);
    auto& transform = registry->get<Transform>(entity);
    auto& movableActor = registry->get<MoveableActor>(entity);
    for (auto n : path) movableActor.globalPath.emplace_back(n);
    transform.direction = Vector3Normalize(Vector3Subtract(movableActor.globalPath.front(), transform.position));

    if (initialMove)
    {
    	transform.onStartMovement.publish(entity);
    }
}

bool AlmostEquals(Vector3 a, Vector3 b)
{
	std::tuple<int, int, int> a1 = { std::round(a.x), std::round(a.y), std::round(a.z) };
	std::tuple<int, int, int> b1 = { std::round(b.x), std::round(b.y), std::round(b.z) };
    return a1 == b1;
}

void ActorMovementSystem::DrawDebug() const
{
    auto view = registry->view<MoveableActor, Transform>();
    for (auto& entity: view) 
    {
        auto &actor = registry->get<MoveableActor>(entity);
        if (actor.globalPath.empty()) continue;
        auto &transform = registry->get<Transform>(entity);
        if (!actor.globalPath.empty())
        {
            for (auto p : actor.globalPath)
            {
                DrawCube(p, 1, 1, 1, GREEN);
            }
        }
    }

    for (auto& ray: debugRays) 
    {
        DrawLine3D(ray.position, Vector3Add(ray.position, Vector3Multiply(ray.direction, {5, 1, 5})), RED);
    }
}

void ActorMovementSystem::Update()
{
	debugRays.erase(debugRays.begin(), debugRays.end());
    auto view = registry->view<MoveableActor, Transform>();
    for (auto& entity: view)
    {
        auto& moveableActor = registry->get<MoveableActor>(entity);
        if (moveableActor.globalPath.empty()) continue;

        const auto& actorCollideable = registry->get<Collideable>(entity);
        navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, false);

        auto& actorTrans = registry->get<Transform>(entity);
    	auto nextPointDist = Vector3Distance(moveableActor.globalPath.front(), actorTrans.position);


		// TODO: Works when I check if "back" is occupied, but not when I check if "front" is occupied. Why?
		// The idea is to check whether the next square is occupied, and if it is, then recalculate the path.
        if (!navigationGridSystem->CheckBoundingBoxAreaUnoccupied(moveableActor.globalPath.back(), actorCollideable.worldBoundingBox))
        {
			PathfindToLocation(entity, moveableActor.globalPath.back(), false);
            navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true, entity);
			continue;
        }

    	if (nextPointDist < 0.5f) // Destination reached
        {
	        moveableActor.globalPath.pop_front();
            if (moveableActor.globalPath.empty())
            {
                actorTrans.onFinishMovement.publish(entity);
            	navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true, entity);
                continue;
            }
        }

        float avoidanceDistance = 10;
        GridSquare actorIndex;
        navigationGridSystem->WorldToGridSpace(actorTrans.position, actorIndex);
        NavigationGridSquare* hitCell = navigationGridSystem->CastRay(
            actorIndex.row, 
            actorIndex.col, 
            { actorTrans.direction.x, actorTrans.direction.z }, 
            avoidanceDistance);
        
        if (hitCell != nullptr)
        {
            auto& hitTransform = registry->get<Transform>(hitCell->occupant);
	        if (moveableActor.lastHitActor != entity || !AlmostEquals(hitTransform.position, moveableActor.hitActorLastPos))
	        {
	            moveableActor.lastHitActor = entity;
	            moveableActor.hitActorLastPos = hitTransform.position;

	            auto& hitCol = registry->get<Collideable>(hitCell->occupant);

	            if (Vector3Distance(hitTransform.position, actorTrans.position) < Vector3Distance(moveableActor.globalPath.back(), actorTrans.position))
	            {
					PathfindToLocation(entity, moveableActor.globalPath.back(), false);
	                hitCol.debugDraw = true;
	            }
	        }
        }

        actorTrans.direction = Vector3Normalize(Vector3Subtract(moveableActor.globalPath.front(), actorTrans.position));
    	// Calculate rotation angle based on direction
    	float angle = atan2f(actorTrans.direction.x, actorTrans.direction.z) * RAD2DEG;
    	actorTrans.rotation.y = angle;
	    actorTrans.position.x = actorTrans.position.x + actorTrans.direction.x * actorTrans.movementSpeed;
	    actorTrans.position.z = actorTrans.position.z + actorTrans.direction.z * actorTrans.movementSpeed;
	    actorTrans.onPositionUpdate.publish(entity);
        navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true, entity);
    }
}

ActorMovementSystem::ActorMovementSystem(entt::registry* _registry, CollisionSystem* _collisionSystem, NavigationGridSystem* _navigationGridSystem) :
    BaseSystem<MoveableActor>(_registry), collisionSystem(_collisionSystem), navigationGridSystem(_navigationGridSystem)
{
}

}
