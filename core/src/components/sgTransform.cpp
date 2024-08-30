//
// Created by Steve Wheeler on 03/05/2024.
//

#include "sgTransform.hpp"
#include "raymath.h"

namespace sage
{
    Matrix sgTransform::GetMatrixNoRot() const
    {
        Matrix trans = MatrixTranslate(m_positionWorld.x, m_positionWorld.y, m_positionWorld.z);
        Matrix _scale = MatrixScale(m_scale, m_scale, m_scale);
        // Matrix rot = MatrixRotateXYZ({DEG2RAD*transform->rotation.x, DEG2RAD*transform->rotation.y,
        // DEG2RAD*transform->rotation.z});
        return MatrixMultiply(trans, _scale);
    }

    Matrix sgTransform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(m_positionWorld.x, m_positionWorld.y, m_positionWorld.z);
        Matrix _scale = MatrixScale(m_scale, m_scale, m_scale);
        Matrix rot = MatrixRotateXYZ({DEG2RAD * m_rotation.x, DEG2RAD * m_rotation.y, DEG2RAD * m_rotation.z});
        return MatrixMultiply(MatrixMultiply(trans, rot), _scale);
    }

    Vector3 sgTransform::forward() const
    {
        Matrix matrix = GetMatrix();
        Vector3 forward = {matrix.m8, matrix.m9, matrix.m10};
        return Vector3Normalize(forward);
    }

    const Vector3& sgTransform::GetWorldPos() const
    {
        return m_positionWorld;
    }

    const Vector3& sgTransform::GetRotation() const
    {
        return m_rotation;
    }

    float sgTransform::GetScale() const
    {
        return m_scale;
    }

    void sgTransform::SetPosition(const Vector3& position)
    {
        m_positionWorld = position;
        if (m_parent)
        {
            m_positionLocal = Vector3Subtract(m_positionWorld, m_parent->GetWorldPos());
        }
        else
        {
            m_positionLocal = m_positionWorld;
        }
        updateChildren();

        onPositionUpdate.publish(self);
    }

    void sgTransform::updateChildren()
    {
        for (auto* child : m_children)
        {
            child->SetPosition(Vector3Add(m_positionWorld, child->GetWorldPos()));
        }
    }

    void sgTransform::SetRotation(const Vector3& rotation)
    {
        m_rotation = rotation;
    }

    void sgTransform::SetScale(float scale)
    {
        m_scale = scale;
    }

    sgTransform* sgTransform::GetParent()
    {
        return m_parent;
    }

    const std::vector<sgTransform*>& sgTransform::GetChildren()
    {
        return m_children;
    }

    void sgTransform::SetParent(sgTransform* newParent)
    {
        m_parent = newParent;
    }

    void sgTransform::AddChild(sgTransform* newChild)
    {
        newChild->SetParent(this);
        m_children.push_back(newChild);
        newChild->SetPosition(Vector3Add(m_positionWorld, newChild->GetWorldPos()));
    }

    sgTransform::sgTransform(entt::entity _self) : self(_self)
    {
    }
} // namespace sage
