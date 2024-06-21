//
// Created by Steve Wheeler on 21/02/2024.
//

#include "ActorMovementSystem.hpp"

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

void ActorMovementSystem::updateMoveTowardsTransforms()
{
    debugRays.erase(debugRays.begin(), debugRays.end());
    auto view = registry->view<MoveableActor, Transform>();
    for (auto& entity: view)
    {
        auto& actor = registry->get<MoveableActor>(entity);
        if (actor.globalPath.empty()) continue;
        auto& transform = registry->get<Transform>(entity);
        
        auto& path = actor.localPath.empty() ? actor.globalPath : actor.localPath;
        transform.direction = Vector3Normalize(Vector3Subtract(path.front(), transform.position));

        auto distance = Vector3Distance(path.front(), transform.position);
        if (distance < 0.5f)
        {
            path.pop_front();
            if (actor.globalPath.empty())
            {
                transform.onFinishMovement.publish(entity);
                continue;
            }
        }
        
        Ray ray;
        float avoidanceDistance = 1.5;
        ray.direction = Vector3Multiply(transform.direction, { avoidanceDistance, 1, avoidanceDistance });
        ray.direction.y = 1.0f;
        ray.position = transform.position;
        ray.position.y = 1.0f;
        debugRays.push_back(ray);
        auto col = collisionSystem->GetCollisionsWithRay(entity, ray, CollisionLayer::BOYD);

        if (!col.empty())
        {
            auto& hitTransform = registry->get<Transform>(col.at(0).collidedEntityId);
            BoundingBox hitBB = col.at(0).collidedBB;

            // TODO: Make sure collision boxes do not overlap
            
            if (Vector3Distance(hitTransform.position, transform.position) < distance)
            {
                auto& hitCollidable = registry->get<Collideable>(col.at(0).collidedEntityId);
                hitCollidable.debugDraw = true;

                auto localPath = navigationGridSystem->ResolveLocalObstacle(entity, hitBB, transform.direction);
                
//                auto localPath = navigationGridSystem->PathfindAvoidLocalObstacle(entity, hitBB, transform.position, actor.globalPath.front());
//                {
//                    std::deque<Vector3> empty;
//                    std::swap(actor.localPath, empty);
//                }
                for (auto & it : std::ranges::reverse_view(localPath))
                {
                    actor.localPath.push_front(it);
                }
                
                //transform.direction = Vector3Normalize(Vector3Subtract(actor.localPath.front(), transform.position));
                continue;
            }

            if (actor.globalPath.size() == 1 && CheckCollisionBoxSphere(hitBB, actor.globalPath.front(), 0.1f))
            {
                
                // Destination occupied
                navigationGridSystem->MarkSquareOccupied(hitBB);
                if (!actor.localPath.empty())
                {
                    std::deque<Vector3> empty;
                    std::swap(actor.localPath, empty);
                }

                auto frontCopy = actor.globalPath.front();
                auto newDestination = navigationGridSystem->FindNextBestLocation(entity, { col.at(0).rlCollision.point.x, col.at(0).rlCollision.point.y });
                actor.globalPath.pop_front();
                actor.globalPath.push_front({ newDestination.x, frontCopy.y, newDestination.y });

                navigationGridSystem->MarkSquareOccupied(hitBB, false);
            }
        }
        
        // Calculate rotation angle based on direction
        float angle = atan2f(transform.direction.x, transform.direction.z) * RAD2DEG;
        transform.rotation.y = angle;
        transform.position.x = transform.position.x + transform.direction.x * transform.movementSpeed;
        //transform->position.y = dy * 0.5f;
        transform.position.z = transform.position.z + transform.direction.z * transform.movementSpeed;
        transform.onPositionUpdate.publish(entity);
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
