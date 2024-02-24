//
// Created by Steve Wheeler on 21/02/2024.
//

#include "TransformSystem.hpp"

namespace sage
{
    void TransformSystem::SetComponent(EntityID entityId, Transform newTransform)
    {
        components.at(entityId)->position = newTransform.position;
    }
}
