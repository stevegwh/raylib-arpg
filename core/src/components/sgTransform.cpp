//
// Created by Steve Wheeler on 03/05/2024.
//

#include "sgTransform.hpp"
#include "raymath.h"
#include "slib.hpp"

namespace sage
{
    Matrix sgTransform::GetMatrixNoRot() const
    {
        Matrix trans = MatrixTranslate(m_positionWorld.x, m_positionWorld.y, m_positionWorld.z);
        Matrix _scale = MatrixScale(m_scale.x, m_scale.y, m_scale.z);
        return MatrixMultiply(trans, _scale);
    }

    Matrix sgTransform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(m_positionWorld.x, m_positionWorld.y, m_positionWorld.z);
        Matrix _scale = MatrixScale(m_scale.x, m_scale.y, m_scale.z);
        Matrix rot = MatrixMultiply(
            MatrixMultiply(MatrixRotateZ(DEG2RAD * m_rotationWorld.z), MatrixRotateY(DEG2RAD * m_rotationWorld.y)),
            MatrixRotateX(DEG2RAD * m_rotationWorld.x));
        return MatrixMultiply(MatrixMultiply(trans, rot), _scale);
    }

    Vector3 sgTransform::forward() const
    {
        Matrix matrix = GetMatrix();
        Vector3 forward = {matrix.m8, matrix.m9, matrix.m10};
        return Vector3Normalize(forward);
    }

    const Vector3& sgTransform::GetLocalPos() const
    {
        return m_positionLocal;
    }

    const Vector3& sgTransform::GetWorldPos() const
    {
        return m_positionWorld;
    }

    const Vector3& sgTransform::GetWorldRot() const
    {
        return m_rotationWorld;
    }

    const Vector3& sgTransform::GetLocalRot() const
    {
        return m_rotationLocal;
    }

    const Vector3& sgTransform::GetScale() const
    {
        return m_scale;
    }

    void sgTransform::updateChildrenPos()
    {
        for (auto* child : m_children)
        {
            child->SetPosition(Vector3Add(m_positionWorld, child->GetLocalPos()));
        }
    }

    void sgTransform::updateChildrenRot()
    {
        for (auto* child : m_children)
        {
            child->SetRotation(Vector3Add(m_rotationWorld, child->GetLocalRot()));
            // TODO: Scale
        }
    }

    void sgTransform::SetLocalPos(const Vector3& position)
    {
        m_positionLocal = position;
        if (!m_parent) return;
        SetPosition(Vector3Add(m_parent->m_positionWorld, m_positionLocal));
    }

    void sgTransform::SetLocalRot(const Quaternion& rotation)
    {
        Vector3 rot = QuaternionToEuler(rotation);
        rot = Vector3MultiplyByValue(rot, RAD2DEG); // raylib gives this back in rad
        SetLocalRot(rot);
    }

    void sgTransform::SetLocalRot(const Vector3& rotation)
    {
        m_rotationLocal = rotation;
        if (!m_parent)
        {
            SetRotation(m_rotationLocal);
        }
        else
        {
            SetRotation(Vector3Add(m_parent->m_rotationWorld, m_rotationLocal));
        }
    }

    void sgTransform::SetPosition(const Vector3& position)
    {
        if (m_parent)
        {
            m_positionWorld = position;
        }
        else
        {
            m_positionLocal = position;
            m_positionWorld = position;
        }
        updateChildrenPos();

        onPositionUpdate.publish(self);
    }

    void sgTransform::SetRotation(const Vector3& rotation)
    {
        if (m_parent)
        {
            m_rotationWorld = rotation;
        }
        else
        {
            m_rotationLocal = rotation;
            m_rotationWorld = rotation;
        }
        updateChildrenRot();
    }

    void sgTransform::SetScale(const Vector3& scale)
    {
        m_scale = scale;
    }

    void sgTransform::SetScale(float scale)
    {
        m_scale = {scale, scale, scale};
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
        if (m_parent)
        {
            // Remove this from old parent's children list
            auto it = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
            if (it != m_parent->m_children.end()) m_parent->m_children.erase(it);
        }

        m_parent = newParent;

        if (m_parent)
        {
            m_parent->m_children.push_back(this);
            // Recalculate local position based on current world position and new parent
            m_positionLocal = Vector3Subtract(m_positionWorld, m_parent->GetWorldPos());
            m_rotationLocal = Vector3Subtract(m_rotationWorld, m_parent->GetWorldRot());
        }
        else
        {
            // If no parent, local = world
            m_positionLocal = m_positionWorld;
            m_rotationLocal = m_rotationWorld;
        }
    }

    void sgTransform::AddChild(sgTransform* newChild)
    {
        newChild->SetParent(this);
    }

    sgTransform::sgTransform(entt::entity _self) : self(_self)
    {
        m_positionLocal = Vector3Zero();
        m_positionWorld = Vector3Zero();
        m_rotationWorld = Vector3Zero();
        m_rotationLocal = Vector3Zero();
        m_scale = Vector3{1, 1, 1};
    }
} // namespace sage
