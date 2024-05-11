//
// Created by Steve Wheeler on 21/02/2024.
//

#include "TransformSystem.hpp"

#include "../Application.hpp"
#include "../../utils/Serializer.hpp"

namespace sage
{

void TransformSystem::PathfindToLocation(const entt::entity& entityId, const std::vector<Vector3>& path)
{
    auto& transform = registry->get<Transform>(entityId);
    // Prune existing move commands
    for (auto it = moveTowardsTransforms.begin(); it != moveTowardsTransforms.end();)
    {
        if (it->second == &transform)
        {
            it = moveTowardsTransforms.erase(it);
            continue;
        }
        ++it;
    }
    
    // Clear queue of previous commands
    std::queue<Vector3> empty;
    std::swap(transform.targets, empty);

    for (auto n : path) transform.targets.emplace(n);
    transform.direction = Vector3Normalize(Vector3Subtract(transform.targets.front(), transform.position));
    moveTowardsTransforms.emplace_back(entityId, &transform);
    transform.onStartMovement.publish(entityId);
}

void TransformSystem::DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data)
{
}

void TransformSystem::Update()
{
    for (auto it = moveTowardsTransforms.begin(); it != moveTowardsTransforms.end();) 
    {
        const auto& transform = it->second;

        if (Vector3Distance(transform->targets.front(), transform->position) < 0.5f)
        {
            transform->targets.pop();
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

        transform->position.x = transform->position.x + transform->direction.x * 0.35f;
        //transform->position.x = dy * 0.5f;
        transform->position.z = transform->position.z + transform->direction.z * 0.35f;
        transform->onPositionUpdate.publish(it->first);
        ++it;
    }
}

TransformSystem::TransformSystem(entt::registry* _registry) :
    BaseSystem<Transform>(_registry)
{
}

}
