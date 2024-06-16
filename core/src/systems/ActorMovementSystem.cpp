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

    // Clear queue of previous commands
    std::deque<Vector3> empty;
    std::swap(transform.targets, empty);
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
    for (auto n : path) transform.targets.emplace_back(n);
    transform.direction = Vector3Normalize(Vector3Subtract(transform.targets.front(), transform.position));
    moveTowardsTransforms.emplace_back(entity, &transform);
    transform.onStartMovement.publish(entity);
}

void ActorMovementSystem::updateMoveTowardsTransforms()
{
    for (auto it = moveTowardsTransforms.begin(); it != moveTowardsTransforms.end();)
    {
        const auto& transform = it->second;
        auto distance = Vector3Distance(transform->targets.front(), transform->position);

        if (distance < 0.5f)
        {
            transform->targets.pop_front();
            if (transform->targets.empty())
            {
                transform->onFinishMovement.publish(it->first);
                it = moveTowardsTransforms.erase(it);
                continue;
            }
            transform->direction = Vector3Normalize(Vector3Subtract(transform->targets.front(), transform->position));
        }

        // Calculate rotation angle based on direction
        float angle = atan2f(transform->direction.x, transform->direction.z) * RAD2DEG;
        transform->rotation.y = angle;

        // TODO: Temporary. Working on boyd avoidance.
        // Move
        // Raycast in direction.
        // Check collision
        // Avoid or not
        Ray ray;
        ray.position = transform->position;
        ray.position.y = 0.5f;
        float avoidanceDistance = 0.5;
        ray.direction = Vector3Multiply(transform->direction, { avoidanceDistance, 1, avoidanceDistance });
        ray.direction.y = 0.5f;
        auto col = collisionSystem->GetCollisionsWithRay(ray, CollisionLayer::BOYD);

        if (col.size() > 1) // First collision will be this unit
        {
            auto& hitTransform = registry->get<Transform>(col.at(1).collidedEntityId);
            if (Vector3Distance(hitTransform.position, transform->position) < distance)
            {
                std::cout << "We have hit something! Avoid! \n";
                BoundingBox hitBB = col.at(1).collidedBB;
                auto path = navigationGridSystem->PathfindAvoidLocalObstacle(it->first, hitBB, transform->position, transform->targets.front());
                
                for (auto & it2 : std::ranges::reverse_view(path))
                {
                    transform->targets.push_front(it2);
                }
                ++it;
                continue;
            }
            // TODO
            // Take the collideable's bounding box and mark the grid squares that it occupies as "occupied" (temporarily, somehow)
            // Use dijkstra's algo, favouring grid squares that are not in the direction the collided object is moving in (move towards their rear)
            // Merge the local avoidance path with the global path somehow
        }
        // ---
        transform->position.x = transform->position.x + transform->direction.x * transform->movementSpeed;
        //transform->position.y = dy * 0.5f;
        transform->position.z = transform->position.z + transform->direction.z * transform->movementSpeed;
        transform->onPositionUpdate.publish(it->first);

        ++it;
    }
}


void ActorMovementSystem::Update()
{
    updateMoveTowardsTransforms();
}

ActorMovementSystem::ActorMovementSystem(entt::registry* _registry, CollisionSystem* _collisionSystem, NavigationGridSystem* _navigationGridSystem) :
    BaseSystem<Transform>(_registry), collisionSystem(_collisionSystem), navigationGridSystem(_navigationGridSystem)
{
}

}
