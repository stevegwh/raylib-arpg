//
// Created by Steve Wheeler on 02/05/2024.
//

#include "Collideable.hpp"
#include "../Application.hpp"

namespace sage
{

Collideable::Collideable(BoundingBox _boundingBox) :
    localBoundingBox(_boundingBox), worldBoundingBox(_boundingBox)
{}

}
