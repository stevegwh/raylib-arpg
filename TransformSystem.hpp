//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <map>
#include <memory>

#include "Transform.hpp"
#include "BaseSystem.hpp"

#include "raymath.h"

namespace sage
{
class TransformSystem : public BaseSystem<Transform>
{
    std::vector<Transform*> moveTowardsTransforms;
public:
    // TODO: Overload this so you can just update one field at a time if needed
    void SetComponent(EntityID entityId, Transform newTransform);
    void MoveToLocation(EntityID entityId, Vector3 location);
    void Update();
};
}
