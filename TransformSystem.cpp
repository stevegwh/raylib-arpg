//
// Created by Steve Wheeler on 21/02/2024.
//

#include "TransformSystem.hpp"

namespace sage
{
    void TransformSystem::AddTransform(Transform& transform)
    {
        transforms.emplace(transform.entityId, std::make_unique<Transform>(transform));
    }
    
    const Transform& TransformSystem::GetComponent(EntityID entityId)
    {
        return *transforms.at(entityId);
    }
}
