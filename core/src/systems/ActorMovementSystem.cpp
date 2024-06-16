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
    
    auto& transform = registry->get<Transform>(entity);

    // Prune existing move commands
    for (auto it = moveTowardsTransforms.begin(); it != moveTowardsTransforms.end();)
    {
        if (it->second == &transform) // TODO: Surely this just needs to be done once?
        {
            it = moveTowardsTransforms.erase(it);
            continue;
        }
        ++it;
    }

    auto& actor = registry->get<MoveableActor>(entity);
    // Clear queue of previous commands
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
    auto view = registry->view<MoveableActor, Transform>();
    for (auto& entity: view)
    {
        auto& actor = registry->get<MoveableActor>(entity);
        if (actor.globalPath.empty()) continue;
        auto& transform = registry->get<Transform>(entity);
        
        if (!actor.localPath.empty())
        {
            
            auto distance = Vector3Distance(actor.localPath.front(), transform.position);
            if (distance < 0.5f)
            {
                actor.localPath.pop_front();
                if (actor.localPath.empty()) continue;
                transform.direction = Vector3Normalize(Vector3Subtract(actor.localPath.front(), transform.position));
            }
        }
        else
        {
            auto distance = Vector3Distance(actor.globalPath.front(), transform.position);
            if (distance < 0.5f)
            {
                actor.globalPath.pop_front();
                if (actor.globalPath.empty())
                {
                    transform.onFinishMovement.publish(entity);
                    continue;
                }
                transform.direction = Vector3Normalize(Vector3Subtract(actor.globalPath.front(), transform.position));
            }
            
            Ray ray;
            ray.position = transform.position;
            ray.position.y = 0.5f;
            float avoidanceDistance = 1;
            ray.direction = Vector3Multiply(transform.direction, { avoidanceDistance, 1, avoidanceDistance });
            ray.direction.y = 0.5f;
            auto col = collisionSystem->GetCollisionsWithRay(ray, CollisionLayer::BOYD);

            if (col.size() > 1) // First collision will be this unit
            {
                auto& hitTransform = registry->get<Transform>(col.at(1).collidedEntityId);
                if (Vector3Distance(hitTransform.position, transform.position) < distance)
                {
                    BoundingBox hitBB = col.at(1).collidedBB;
                    auto& c = registry->get<Collideable>(col.at(1).collidedEntityId);
                    c.debugDraw = true;
                    auto path = navigationGridSystem->PathfindAvoidLocalObstacle(entity, hitBB, transform.position, actor.globalPath.front());

                    for (auto & it : std::ranges::reverse_view(path))
                    {
                        actor.localPath.push_front(it);
                    }
                    continue;
                }
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

void ActorMovementSystem::DebugDraw() const
{
    auto view = registry->view<MoveableActor, Transform>();
    for (auto& entity: view) 
    {
        auto &actor = registry->get<MoveableActor>(entity);
        if (actor.globalPath.empty()) continue;
        auto &transform = registry->get<Transform>(entity);
        if (!actor.localPath.empty())
        {
            for (auto p : actor.localPath) 
            {
                DrawCube(p, 1, 1, 1, RED);
            }
        }
        if (!actor.globalPath.empty())
        {
            for (auto p : actor.globalPath)
            {
                DrawCube(p, 1, 1, 1, GREEN);
            }
        }
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
