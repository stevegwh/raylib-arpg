//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include <map>
#include "Transform.hpp"
#include <memory>

namespace sage
{
    class TransformSystem
    {
        std::map<EntityID, std::unique_ptr<Transform>> transforms;
    public:
        void AddTransform(Transform& transform);
        const Transform& GetComponent(EntityID entityId);
    };
}
