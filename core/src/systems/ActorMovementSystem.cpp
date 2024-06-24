//
// Created by Steve Wheeler on 21/02/2024.
//

#include "ActorMovementSystem.hpp"

#include "../components/NavigationGridSquare.hpp"

#include "../../utils/Serializer.hpp"

#include <tuple>
#include <ranges>

namespace sage
{

void ActorMovementSystem::PruneMoveCommands(const entt::entity& entity)
{
    auto& actor = registry->get<MoveableActor>(entity);
    {
        std::deque<Vector3> empty;
        std::swap(actor.globalPath, empty);
    }

}

void ActorMovementSystem::CancelMovement(const entt::entity& entity)
{
    PruneMoveCommands(entity);
    auto& transform = registry->get<Transform>(entity);
    transform.onMovementCancel.publish(entity);
}


void ActorMovementSystem::PathfindToLocation(const entt::entity& entity, const std::vector<Vector3>& path) // TODO: Pathfinding/movement needs some sense of movement speed.
{
    PruneMoveCommands(entity);
    auto& transform = registry->get<Transform>(entity);
    auto& actor = registry->get<MoveableActor>(entity);
    for (auto n : path) actor.globalPath.emplace_back(n);
    transform.direction = Vector3Normalize(Vector3Subtract(actor.globalPath.front(), transform.position));
    //moveTowardsTransforms.emplace_back(entity, &transform);
    transform.onStartMovement.publish(entity);

}

bool AlmostEquals(Vector3 a, Vector3 b)
{
	std::tuple<int, int, int> a1 = { std::round(a.x), std::round(a.y), std::round(a.z) };
	std::tuple<int, int, int> b1 = { std::round(b.x), std::round(b.y), std::round(b.z) };
    return a1 == b1;
}

void ActorMovementSystem::updateMoveTowardsTransforms()
{
    debugRays.erase(debugRays.begin(), debugRays.end());
    auto view = registry->view<MoveableActor, Transform>();
    for (auto& entity: view)
    {
        auto& moveableActor = registry->get<MoveableActor>(entity);
        if (moveableActor.globalPath.empty()) continue;

        const auto& actorCollideable = registry->get<Collideable>(entity);
        navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, false);

        auto& path = moveableActor.globalPath;

        auto& actorTrans = registry->get<Transform>(entity);


    	auto nextPointDist = Vector3Distance(path.front(), actorTrans.position);
    	if (nextPointDist < 0.5f)
        {
	        if (!navigationGridSystem->CheckSingleSquareOccupied(path.front()))
	        {
		        path.pop_front();
	            if (moveableActor.globalPath.empty())
	            {
	                actorTrans.onFinishMovement.publish(entity);
            		navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true, entity);
	                continue;
	            }
	        }
            else
            {
            	auto newPath = navigationGridSystem->AStarPathfind(entity, actorTrans.position, moveableActor.globalPath.back());                
                std::deque<Vector3> empty;
                std::swap(moveableActor.globalPath, empty);
                for (auto& node : newPath)
                {
	                moveableActor.globalPath.push_back(node);
                }
                continue;
            }

        }

        float avoidanceDistance = 10;
        Vector2 actorIndex;
        navigationGridSystem->WorldToGridSpace(actorTrans.position, actorIndex);
        NavigationGridSquare* hitCell = navigationGridSystem->CastRay(actorIndex.y, actorIndex.x, { actorTrans.direction.x, actorTrans.direction.z }, avoidanceDistance);
        
        if (hitCell != nullptr)
        {
            auto& hitTransform = registry->get<Transform>(hitCell->occupant);
	        if (AlmostEquals(hitTransform.position, moveableActor.hitActorLastPos)) // TODO: this will be true if 0,0
	        {
                continue;
	        }
            moveableActor.hitActorLastPos = hitTransform.position;

            auto& hitCol = registry->get<Collideable>(hitCell->occupant);

            if (Vector3Distance(hitTransform.position, actorTrans.position) < nextPointDist)
            {

                // TODO: This will get called despite colliding with the same entity again and again
                // Could just store a "lastHitActor" variable and see if (hitCell->occupant = lastHitActor && hitTransform.position != lastHitActorPos) and see if the occupant has moved
                auto newPath = navigationGridSystem->AStarPathfind(entity, actorTrans.position, moveableActor.globalPath.back());                
                std::deque<Vector3> empty;
                std::swap(moveableActor.globalPath, empty);
                for (auto& node : newPath)
                {
	                moveableActor.globalPath.push_back(node);
                }

                hitCol.debugDraw = true;
            }
        }

        actorTrans.direction = Vector3Normalize(Vector3Subtract(path.front(), actorTrans.position));
    	// Calculate rotation angle based on direction
    	float angle = atan2f(actorTrans.direction.x, actorTrans.direction.z) * RAD2DEG;
    	actorTrans.rotation.y = angle;
	    actorTrans.position.x = actorTrans.position.x + actorTrans.direction.x * actorTrans.movementSpeed;
	    actorTrans.position.z = actorTrans.position.z + actorTrans.direction.z * actorTrans.movementSpeed;
	    actorTrans.onPositionUpdate.publish(entity);
        navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true, entity);
    }
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
    updateMoveTowardsTransforms();
}

ActorMovementSystem::ActorMovementSystem(entt::registry* _registry, CollisionSystem* _collisionSystem, NavigationGridSystem* _navigationGridSystem) :
    BaseSystem<MoveableActor>(_registry), collisionSystem(_collisionSystem), navigationGridSystem(_navigationGridSystem)
{
}

}
