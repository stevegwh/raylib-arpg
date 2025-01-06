//
// Created by Steve Wheeler on 02/05/2024.
//

#include "Collideable.hpp"

#include "components/sgTransform.hpp"

#include "raymath.h"

namespace sage
{
    void Collideable::OnTransformUpdate(entt::entity self)
    {
        auto& trans = registry->get<sgTransform>(self);
        Matrix mat = trans.GetMatrixNoRot(); // AABB, so no rotation
        SetWorldBoundingBox(mat);
    }

    void Collideable::SetWorldBoundingBox(Matrix mat)
    {
        auto bb = localBoundingBox;
        bb.min = Vector3Transform(bb.min, mat);
        bb.max = Vector3Transform(bb.max, mat);
        worldBoundingBox = bb;
    }

    void Collideable::Enable()
    {
        active = true;
    }

    void Collideable::Disable()
    {
        active = false;
    }

    Collideable::Collideable(const BoundingBox& _localBoundingBox, const Matrix& worldMatrix)
        : localBoundingBox(_localBoundingBox)
    {
        SetWorldBoundingBox(worldMatrix);
    }

    // Applies the world matrix of this entity's transform and subscribes to its updates
    Collideable::Collideable(entt::registry* _registry, entt::entity _self, BoundingBox _localBoundingBox)
        : registry(_registry), localBoundingBox(_localBoundingBox), worldBoundingBox(_localBoundingBox)
    {
        assert(registry->any_of<sgTransform>(_self));
        auto& transform = registry->get<sgTransform>(_self);
        transform.onPositionUpdate.Subscribe([this](entt::entity entity) { OnTransformUpdate(entity); });
        SetWorldBoundingBox(transform.GetMatrix());
    }
} // namespace sage
