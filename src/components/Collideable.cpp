//
// Created by Steve Wheeler on 02/05/2024.
//

#include "Collideable.hpp"
#include "../GameManager.hpp"

namespace sage
{

void Collideable::onTransformUpdate(entt::entity entity)
{
    Matrix mat = ECS->transformSystem->GetMatrixNoRot(entity);
    auto bb = localBoundingBox;
    bb.min = Vector3Transform(bb.min, mat);
    bb.max = Vector3Transform(bb.max, mat);
    worldBoundingBox = bb;
}

Collideable::Collideable(BoundingBox _boundingBox) :
    localBoundingBox(_boundingBox), worldBoundingBox(_boundingBox)
{}

}
