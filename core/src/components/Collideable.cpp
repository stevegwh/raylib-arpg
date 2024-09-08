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

    Collideable::Collideable(BoundingBox _localBoundingBox, Matrix mat) : localBoundingBox(_localBoundingBox)
    {
        SetWorldBoundingBox(mat);
    }

    // Applies the world matrix of this entity's transform and subscribes to its updates
    Collideable::Collideable(entt::registry* _registry, entt::entity _self, BoundingBox _localBoundingBox)
        : registry(_registry), localBoundingBox(_localBoundingBox), worldBoundingBox(_localBoundingBox)
    {
        assert(registry->any_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(_self);
        entt::sink sink{transform.onPositionUpdate};
        sink.connect<&Collideable::OnTransformUpdate>(this);
        SetWorldBoundingBox(transform.GetMatrix());
    }
} // namespace sage
