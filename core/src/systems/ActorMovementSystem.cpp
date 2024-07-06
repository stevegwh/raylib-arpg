//
// Created by Steve Wheeler on 21/02/2024.
//

#include "ActorMovementSystem.hpp"

#include "../components/NavigationGridSquare.hpp"

#include "../../utils/Serializer.hpp"

#include <tuple>
#include <ranges>

#include "components/ControllableActor.hpp"
#include "components/Renderable.hpp"

bool AlmostEquals(Vector3 a, Vector3 b)
{
	std::tuple<int, int> aInt = { static_cast<int>(a.x), static_cast<int>(a.z) };
	std::tuple<int, int> bInt = { static_cast<int>(b.x), static_cast<int>(b.z) };
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
		auto& transform = registry->get<Transform>(entity);
		auto& moveableActor = registry->get<MoveableActor>(entity);
		moveableActor.destination.reset();
		transform.onMovementCancel.publish(entity);
	}

	void ActorMovementSystem::PathfindToLocation(const entt::entity& entity, const Vector3& destination,
		bool initialMove)
	// TODO: Pathfinding/movement needs some sense of movement speed.
	{
		{
			// If location outside of bounds, then return
			GridSquare tmp;
			if (!navigationGridSystem->WorldToGridSpace(destination, tmp)) return;
		}

		int bounds = 50;
		if (registry->any_of<ControllableActor>(entity))
			// TODO: Why is this only for controllable actors? Shouldn't all actors have pathfinding bounds?
		{
			const auto& controllableActor = registry->get<ControllableActor>(entity);
			bounds = controllableActor.pathfindingBounds;
		}

		const auto& actorCollideable = registry->get<Collideable>(entity);
		navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, false);
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
		auto path = navigationGridSystem->AStarPathfind(entity, actorTrans.position, destination, minRange, maxRange,
			AStarHeuristic::FAVOUR_RIGHT);

		PruneMoveCommands(entity);
		auto& transform = registry->get<Transform>(entity);
		auto& movableActor = registry->get<MoveableActor>(entity);
		for (auto n : path) movableActor.path.emplace_back(n);
		if (!path.empty())
		{
			transform.direction = Vector3Normalize(Vector3Subtract(movableActor.path.front(), transform.position));

			if (initialMove)
			{
				movableActor.destination =
					path.back(); // path.back instead of destination to account for the requested destination being occupied
			}
			transform.onStartMovement.publish(entity);
		}
		// Else? Error. Destination is unreachable.
	}

	void ActorMovementSystem::DrawDebug() const
	{
		auto view = registry->view<MoveableActor, Transform>();
		for (auto& entity : view)
		{
			auto& actor = registry->get<MoveableActor>(entity);
			if (actor.path.empty()) continue;
			auto& transform = registry->get<Transform>(entity);
			if (!actor.path.empty())
			{
				for (auto p : actor.path)
				{
					DrawCube(p, 1, 1, 1, GREEN);
				}
			}
		}

		for (auto& ray : debugRays)
		{
			DrawLine3D(ray.position, Vector3Add(ray.position, Vector3Multiply(ray.direction, { 5, 1, 5 })), RED);
		}

        for (auto& col: debugCollisions) 
        {
            DrawSphere(col.point, 1.5, GREEN);
        }
	}

	void ActorMovementSystem::Update()
	{
		// TODO: Instead of always calling this no matter what... maybe choose to call it (or not) based on the state. So, you can pass in the entity to Update and iterate over a collectioon
		// from another system
		debugRays.erase(debugRays.begin(), debugRays.end());
        debugCollisions.erase(debugCollisions.begin(), debugCollisions.end());
		auto view = registry->view<MoveableActor, Transform>();
		for (auto& entity : view)
		{
            auto& actorTrans = registry->get<Transform>(entity);
			auto& moveableActor = registry->get<MoveableActor>(entity);
			const auto& actorCollideable = registry->get<Collideable>(entity);

			if (moveableActor.destination.has_value())
			{
				if (moveableActor.path.empty()
					|| !AlmostEquals(moveableActor.path.back(), moveableActor.destination.value()))
				{
					if (navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
						moveableActor.destination.value(), actorCollideable.worldBoundingBox))
					{
						// TODO: Not sure how much I like having to mark squares as occupied etc.
						navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, false);
						PathfindToLocation(entity, moveableActor.destination.value(), false);
						navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, true, entity);
					}
				}
			}

			if (moveableActor.path.empty())
			{
				continue;
			}

			navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, false);
            Vector2 frontV2 = { moveableActor.path.front().x, moveableActor.path.front().z };
            Vector2 posV2 = { actorTrans.position.x, actorTrans.position.z };
			auto nextPointDist = Vector2Distance(frontV2, posV2);


			// TODO: Works when I check if "back" is occupied, but not when I check if "front" is occupied. Why?
			// The idea is to check whether the next square is occupied, and if it is, then recalculate the path.
			if (!navigationGridSystem->CheckBoundingBoxAreaUnoccupied(moveableActor.path.front(),
				actorCollideable.worldBoundingBox)
				|| !navigationGridSystem->CheckBoundingBoxAreaUnoccupied(
					moveableActor.path.back(), actorCollideable.worldBoundingBox))
			{
				PathfindToLocation(entity, moveableActor.path.back(), false);
				navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, true, entity);
				continue;
			}
            
			if (nextPointDist < 0.5f) // Destination reached
			{
				if (moveableActor.path.size() == 1 && moveableActor.destination.has_value() && AlmostEquals(
					moveableActor.destination.value(), moveableActor.path.back()))
				{
                    //actorTrans.position.y = cachedY;
					actorTrans.onDestinationReached.publish(entity);
					moveableActor.destination.reset();
				}
				moveableActor.path.pop_front();
				if (moveableActor.path.empty())
				{
					actorTrans.onFinishMovement
						.publish(entity); // Should this be published when it reaches its true destination or just a movement? Maybe have two events for it
					navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, true, entity);
					continue;
				}
			}

			float avoidanceDistance = 10;
			GridSquare actorIndex;
			navigationGridSystem->WorldToGridSpace(actorTrans.position, actorIndex);


			navigationGridSystem->MarkSquaresDebug(moveableActor.debugRay, PURPLE, false);
			NavigationGridSquare* hitCell = navigationGridSystem->CastRay(
				actorIndex.row,
				actorIndex.col,
				{ actorTrans.direction.x, actorTrans.direction.z },
				avoidanceDistance, moveableActor.debugRay);

			if (hitCell != nullptr)
			{
				auto& hitTransform = registry->get<Transform>(hitCell->occupant);
				if (moveableActor.lastHitActor != entity || !AlmostEquals(
					hitTransform.position, moveableActor.hitActorLastPos))
				{
					moveableActor.lastHitActor = entity;
					moveableActor.hitActorLastPos = hitTransform.position;

					auto& hitCol = registry->get<Collideable>(hitCell->occupant);

					if (Vector2Distance({hitTransform.position.x, hitTransform.position.z}, posV2) < Vector2Distance(
                        {moveableActor.path.back().x, moveableActor.path.back().z}, posV2))
					{
						PathfindToLocation(entity, moveableActor.path.back(), false);
						hitCol.debugDraw = true;
					}
				}
			}

//            Ray ray;
//            ray.position = actorTrans.position;
//            ray.position.y = actorCollideable.worldBoundingBox.max.y;
//            ray.direction = { 0, -1, 0 };
//            debugRays.push_back(ray);
//            auto collisions = collisionSystem->GetMeshCollisionsWithRay(entity, ray, CollisionLayer::NAVIGATION);
//            if (!collisions.empty())
//            {
//                auto hitentt = collisions.at(0).collidedEntityId;
//                if (registry->any_of<Renderable>(hitentt))
//                {
//                    auto& name = registry->get<Renderable>(collisions.at(0).collidedEntityId).name;
//                    std::cout << "Hit with object: " << name << std::endl;
//                }
//                else
//                {
//                    auto text = TextFormat("Likely hit floor, with entity ID: %d", collisions.at(0).collidedEntityId);
//                    std::cout << text << std::endl;
//                }
//                
//                
//                //auto newPos = Vector3Subtract(actorCollideable.localBoundingBox.max, collisions.at(0).rlCollision.point);
//                actorTrans.position.y = collisions.at(0).rlCollision.point.y;
//                //debugCollisions.push_back(collisions.at(0).rlCollision);
//                
//            }
//            else
//            {
//                std::cout << "No collision with terrain detected \n";
//            }

			actorTrans.direction = Vector3Normalize(Vector3Subtract(moveableActor.path.front(), actorTrans.position));
			// Calculate rotation angle based on direction
			float angle = atan2f(actorTrans.direction.x, actorTrans.direction.z) * RAD2DEG;
			actorTrans.rotation.y = angle;
			actorTrans.position.x = actorTrans.position.x + actorTrans.direction.x * actorTrans.movementSpeed;
			auto gridSquare = navigationGridSystem->GetGridSquare(actorIndex.row, actorIndex.col);
            actorTrans.position.y = gridSquare->terrainHeight;
			actorTrans.position.z = actorTrans.position.z + actorTrans.direction.z * actorTrans.movementSpeed;
			actorTrans.onPositionUpdate.publish(entity);
			navigationGridSystem->MarkSquareAreaOccupied(actorCollideable.worldBoundingBox, true, entity);
		}
	}

	ActorMovementSystem::ActorMovementSystem(entt::registry* _registry, CollisionSystem* _collisionSystem,
		NavigationGridSystem* _navigationGridSystem) :
		BaseSystem<MoveableActor>(_registry), collisionSystem(_collisionSystem),
		navigationGridSystem(_navigationGridSystem)
	{
	}
}
