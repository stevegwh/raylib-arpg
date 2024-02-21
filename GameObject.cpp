//
// Created by Steve Wheeler on 21/02/2024.
//

#include "GameObject.hpp"
namespace sage
{
    void GameObject::SetPosition(Vector3 _position)
    {
        transform->position = _position;
        collideable->boundingBox = GetMeshBoundingBox(renderable->model.meshes[0]);
        collideable->boundingBox.min = Vector3Add(collideable->boundingBox.min, renderable->transform->position);
        collideable->boundingBox.max = Vector3Add(collideable->boundingBox.max, renderable->transform->position);
    }
    
    const Vector3& GameObject::GetPosition()
    {
        return transform->position;
    }
    
    const BoundingBox& GameObject::GetBoundingBox()
    {
        return collideable->boundingBox;
    }
    
    const sage::Collideable* GameObject::GetCollideable()
    {
        return collideable.get();
    }

    const sage::Renderable* GameObject::GetRenderable()
    {
        return renderable.get();
    }

    const sage::Transform* GameObject::GetTransform()
    {
        return transform.get();
    }
}

