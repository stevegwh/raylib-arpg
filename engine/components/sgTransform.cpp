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

    void sgTransform::SetWorldPos(const Vector3& position)
    {
        m_positionWorld = position;
        if (m_parent == entt::null) m_positionLocal = position;
    }

    void sgTransform::SetWorldRot(const Vector3& rotation)
    {
        m_rotationWorld = rotation;
        if (m_parent == entt::null) m_rotationLocal = rotation;
    }

    void sgTransform::SetWorldScale(const Vector3& scale)
    {
        m_scale = scale;
    }

    entt::entity sgTransform::GetParent() const
    {
        return m_parent;
    }

    const std::vector<entt::entity>& sgTransform::GetChildren() const
    {
        return m_children;
    }

    sgTransform::sgTransform(const sgTransform& rhs)
    {
        m_positionLocal = rhs.m_positionLocal;
        m_rotationLocal = rhs.m_rotationLocal;
        m_positionWorld = rhs.m_positionWorld;
        m_rotationWorld = rhs.m_rotationWorld; // FIXED: was rhs.m_positionWorld
        m_scale = rhs.m_scale;
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
    }

    sgTransform& sgTransform::operator=(const sgTransform& rhs)
    {
        m_positionLocal = rhs.m_positionLocal;
        m_rotationLocal = rhs.m_rotationLocal;
        m_positionWorld = rhs.m_positionWorld;
        m_rotationWorld = rhs.m_rotationWorld;
        m_scale = rhs.m_scale;
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
        return *this; // ADDED: missing return statement
    }

    sgTransform::sgTransform(sgTransform&& rhs) noexcept
    {
        m_positionLocal = rhs.m_positionLocal;
        m_rotationLocal = rhs.m_rotationLocal;
        m_positionWorld = rhs.m_positionWorld;
        m_rotationWorld = rhs.m_rotationWorld;
        m_scale = rhs.m_scale;
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
    }

    // ADDED: Missing move assignment operator
    sgTransform& sgTransform::operator=(sgTransform&& rhs) noexcept
    {
        m_positionLocal = rhs.m_positionLocal;
        m_rotationLocal = rhs.m_rotationLocal;
        m_positionWorld = rhs.m_positionWorld;
        m_rotationWorld = rhs.m_rotationWorld;
        m_scale = rhs.m_scale;
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
        return *this;
    }
} // namespace sage
