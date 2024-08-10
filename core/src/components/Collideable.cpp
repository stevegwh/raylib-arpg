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
        Matrix mat = trans.GetMatrixNoRot();
        SetWorldBoundingBox(mat);
    }

    void Collideable::SetWorldBoundingBox(Matrix mat)
    {
        auto bb = localBoundingBox;
        bb.min = Vector3Transform(bb.min, mat);
        bb.max = Vector3Transform(bb.max, mat);
        worldBoundingBox = bb;
    }

    Collideable::Collideable(BoundingBox _boundingBox)
        : localBoundingBox(_boundingBox), worldBoundingBox(_boundingBox)
    {
    }

    Collideable::Collideable(
        entt::registry* _registry, entt::entity _self, BoundingBox _boundingBox)
        : registry(_registry),
          localBoundingBox(_boundingBox),
          worldBoundingBox(_boundingBox)
    {
        // Link the transform to the collideable
        auto& transform = registry->get<sgTransform>(_self);
        entt::sink sink{transform.onPositionUpdate};
        sink.connect<&Collideable::OnTransformUpdate>(this);
        OnTransformUpdate(_self);
    }
} // namespace sage
