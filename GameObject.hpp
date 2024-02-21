//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "raylib.h"
#include "raymath.h"

#include <memory>

#include "Transform.hpp"
#include "Renderable.hpp"
#include "Collideable.hpp"

namespace sage
{
    class GameObject
    {
        std::unique_ptr<sage::Transform> transform;
        std::unique_ptr<sage::Renderable> renderable;
        std::unique_ptr<sage::Collideable> collideable;
        
    public:
        GameObject(sage::Renderable& _renderable, sage::Transform& _transform) :
        renderable(std::make_unique<Renderable>(_renderable)),
        transform(std::make_unique<Transform>(_transform)),
        collideable(std::make_unique<Collideable>())
        {
            collideable->boundingBox = GetMeshBoundingBox(renderable->model.meshes[0]);
            collideable->boundingBox.min = Vector3Add(collideable->boundingBox.min, renderable->transform->position);
            collideable->boundingBox.max = Vector3Add(collideable->boundingBox.max, renderable->transform->position);
        }
        
        const Vector3& GetPosition();
        const BoundingBox& GetBoundingBox();
        const sage::Collideable* GetCollideable();
        const sage::Renderable* GetRenderable();
        const sage::Transform* GetTransform();

        void SetPosition(Vector3 _position);
    };
}

