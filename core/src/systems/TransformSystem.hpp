//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "../components/Transform.hpp"
#include "BaseSystem.hpp"

#include "raymath.h"
#include "entt/entt.hpp"

#include <map>
#include <memory>
#include <utility>
#include <queue>
#include <vector>

namespace sage
{
class TransformSystem : public BaseSystem<Transform>
{
    std::vector<std::pair<entt::entity, Transform*>> moveTowardsTransforms;
public:
    TransformSystem(entt::registry* _registry);
    void PruneMoveCommands(const entt::entity& entity);
    // TODO: Overload this so you can just update one field at a time if needed
    void PathfindToLocation(const entt::entity& entity, const std::vector<Vector3>& path);
    void MoveToLocation(const entt::entity& entity, Vector3 location);
    void CancelMovement(const entt::entity& entity);
    void Update();
};
}
