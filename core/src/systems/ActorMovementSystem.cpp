//
// Created by Steve Wheeler on 21/02/2024.
//

#include "ActorMovementSystem.hpp"

#include "../components/NavigationGridSquare.hpp"

#include "../../utils/Serializer.hpp"

#include <iostream>
#include <ranges>

namespace sage
{

void ActorMovementSystem::PruneMoveCommands(const entt::entity& entity)
{
    auto& actor = registry->get<MoveableActor>(entity);
    {
        std::deque<Vector3> empty;
        std::swap(actor.localPath, empty);
    }
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

//void ActorMovementSystem::updateMoveTowardsTransforms()
//{
//    debugRays.erase(debugRays.begin(), debugRays.end());
//    auto view = registry->view<MoveableActor, Transform>();
//    for (auto& entity: view)
//    {
//        auto& moveableActor = registry->get<MoveableActor>(entity);
//        if (moveableActor.globalPath.empty()) continue;
//
//        const auto& actorCollideable = registry->get<Collideable>(entity);
//        navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, false);
//
//        auto& path = moveableActor.localPath.empty() ? moveableActor.globalPath : moveableActor.localPath;
//
//        auto& actorTrans = registry->get<Transform>(entity);
//        actorTrans.direction = Vector3Normalize(Vector3Subtract(path.front(), actorTrans.position));
//
//        auto distance = Vector3Distance(path.front(), actorTrans.position);
//        if (distance < 0.5f)
//        {
//            path.pop_front();
//            if (moveableActor.globalPath.empty())
//            {
//                actorTrans.onFinishMovement.publish(entity);
//                continue;
//            }
//        }
//        
//        Ray ray;
//        float avoidanceDistance = 1.5;
//        ray.direction = Vector3Multiply(actorTrans.direction, { avoidanceDistance, 1, avoidanceDistance });
//        ray.direction.y = 1.0f;
//        ray.position = actorTrans.position;
//        ray.position.y = 1.0f;
//        debugRays.push_back(ray);
//
//        auto col = collisionSystem->GetCollisionsWithRay(entity, ray, CollisionLayer::BOYD);
//
//        if (!col.empty())
//        {
//            auto& hitTransform = registry->get<Transform>(col.at(0).collidedEntityId);
//            BoundingBox hitBB = col.at(0).collidedBB;
//
//            // TODO: Make sure collision boxes do not overlap
//            
//            if (Vector3Distance(hitTransform.position, actorTrans.position) < distance)
//            {
//                auto& hitCollidable = registry->get<Collideable>(col.at(0).collidedEntityId);
//                hitCollidable.debugDraw = true;
//
//                //auto localPath = navigationGridSystem->ResolveLocalObstacle(entity, hitBB, transform.direction);
//                
//                auto localPath = navigationGridSystem->PathfindAvoidLocalObstacle(entity, hitBB, actorTrans.position, moveableActor.globalPath.front());
//                {
//                    std::deque<Vector3> empty;
//                    std::swap(moveableActor.localPath, empty);
//                }
//                for (auto & it : std::ranges::reverse_view(localPath))
//                {
//                    moveableActor.localPath.push_front(it);
//                }
//                
//                //transform.direction = Vector3Normalize(Vector3Subtract(actor.localPath.front(), transform.position));
//                continue;
//            }
//
//            if (moveableActor.globalPath.size() == 1 && CheckCollisionBoxSphere(hitBB, moveableActor.globalPath.front(), 0.1f))
//            {
//                
//                // Destination occupied
//                if (!moveableActor.localPath.empty())
//                {
//                    std::deque<Vector3> empty;
//                    std::swap(moveableActor.localPath, empty);
//                }
//
//                auto frontCopy = moveableActor.globalPath.front();
//                auto newDestination = navigationGridSystem->FindNextBestLocation(entity, { col.at(0).rlCollision.point.x, col.at(0).rlCollision.point.y });
//                moveableActor.globalPath.pop_front();
//                moveableActor.globalPath.push_front({ newDestination.x, frontCopy.y, newDestination.y });
//
//            }
//        }
//        
//        // Calculate rotation angle based on direction
//        float angle = atan2f(actorTrans.direction.x, actorTrans.direction.z) * RAD2DEG;
//        actorTrans.rotation.y = angle;
//        actorTrans.position.x = actorTrans.position.x + actorTrans.direction.x * actorTrans.movementSpeed;
//        actorTrans.position.z = actorTrans.position.z + actorTrans.direction.z * actorTrans.movementSpeed;
//        actorTrans.onPositionUpdate.publish(entity);
//        navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true);
//    }
//}

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

        auto& path = moveableActor.localPath.empty() ? moveableActor.globalPath : moveableActor.localPath;

        auto& actorTrans = registry->get<Transform>(entity);
        actorTrans.direction = Vector3Normalize(Vector3Subtract(path.front(), actorTrans.position));
    	// Calculate rotation angle based on direction
    	float angle = atan2f(actorTrans.direction.x, actorTrans.direction.z) * RAD2DEG;
    	actorTrans.rotation.y = angle;

    	auto distance = Vector3Distance(path.front(), actorTrans.position);
    	if (distance < 0.5f)
        {
            path.pop_front();
            if (moveableActor.globalPath.empty())
            {
                actorTrans.onFinishMovement.publish(entity);
            	navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true, entity);
                continue;
            }
        }

        float avoidanceDistance = 10;
        Vector2 actorIndex;
        navigationGridSystem->WorldToGridSpace(actorTrans.position, actorIndex);
        NavigationGridSquare* hitCell{};
        
        if (navigationGridSystem->CastRay(actorIndex.y, actorIndex.x, { actorTrans.direction.x, actorTrans.direction.z }, avoidanceDistance, hitCell))
        {
            auto& hitTransform = registry->get<Transform>(hitCell->occupant);
            auto& hitCol = registry->get<Collideable>(hitCell->occupant);
            BoundingBox hitBB = hitCol.worldBoundingBox;

            if (Vector3Distance(hitTransform.position, actorTrans.position) < distance)
            {
	            {
                    std::deque<Vector3> empty;
                    std::swap(moveableActor.localPath, empty);
                }
                hitCol.debugDraw = true;
            	Vector3 newLocation;
                auto nextBest = navigationGridSystem->FindNextBestLocation(entity, { hitCell->worldPosMin.x, hitCell->worldPosMin.z });
            	navigationGridSystem->GridToWorldSpace(nextBest, newLocation);
                moveableActor.localPath.push_back(newLocation);
            }
        }
        else
        {
		    actorTrans.position.x = actorTrans.position.x + actorTrans.direction.x * actorTrans.movementSpeed;
		    actorTrans.position.z = actorTrans.position.z + actorTrans.direction.z * actorTrans.movementSpeed;
		    actorTrans.onPositionUpdate.publish(entity);
		    navigationGridSystem->MarkSquareOccupied(actorCollideable.worldBoundingBox, true, entity);
        }
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
        if (!actor.localPath.empty())
        {
            for (auto p : actor.localPath)
            {
                DrawCube(p, 0.8, 1.2, 0.8, RED);
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
