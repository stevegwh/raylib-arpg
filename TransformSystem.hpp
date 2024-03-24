//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <map>
#include <memory>
#include <utility>
#include <queue>
#include <vector>

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
    void SetComponent(EntityID entityId, const Transform& newTransform);
    void PathfindToLocation(EntityID entityId, const std::vector<Vector3>& path);
    void MoveToLocation(EntityID entityId, Vector3 location);
    void DeserializeComponents(const std::string& entityId, const std::unordered_map<std::string, std::string>& data);
    void Update();
    TransformSystem() : BaseSystem<Transform>("Transform") {}
};
}
