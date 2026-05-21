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
        Matrix _scale = MatrixScale(m_scaleWorld.x, m_scaleWorld.y, m_scaleWorld.z);
        return MatrixMultiply(trans, _scale);
    }

    Matrix sgTransform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(m_positionWorld.x, m_positionWorld.y, m_positionWorld.z);
        Matrix _scale = MatrixScale(m_scaleWorld.x, m_scaleWorld.y, m_scaleWorld.z);
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
        return m_scaleWorld;
    }

    const Vector3& sgTransform::GetLocalScale() const
    {
        return m_scaleLocal;
    }

    void sgTransform::SetWorldPos(const Vector3& position)
    {
        m_positionWorld = position;
        if (m_parent == entt::null) m_positionLocal = position;
        m_dirty = true;
    }

    void sgTransform::SetWorldRot(const Vector3& rotation)
    {
        m_rotationWorld = rotation;
        if (m_parent == entt::null) m_rotationLocal = rotation;
        m_dirty = true;
    }

    void sgTransform::SetWorldScale(const Vector3& scale)
    {
        m_scaleWorld = scale;
        if (m_parent == entt::null) m_scaleLocal = scale;
        m_dirty = true;
    }

    void sgTransform::SetWorldScale(float scale)
    {
        SetWorldScale(Vector3{scale, scale, scale});
    }

    void sgTransform::SetLocalScale(const Vector3& scale)
    {
        m_scaleLocal = scale;
        if (m_parent == entt::null) m_scaleWorld = scale;
        m_dirty = true;
    }

    void sgTransform::SetLocalScale(float scale)
    {
        SetLocalScale(Vector3{scale, scale, scale});
    }

    void sgTransform::SetLocalRot(const Quaternion& rotation)
    {
        Vector3 rot = QuaternionToEuler(rotation);
        rot = Vector3MultiplyByValue(rot, RAD2DEG);
        SetLocalRot(rot);
    }

    void sgTransform::SetLocalPos(const Vector3& position)
    {
        m_positionLocal = position;
        if (m_parent == entt::null) m_positionWorld = position;
        m_dirty = true;
    }

    void sgTransform::SetLocalRot(const Vector3& rotation)
    {
        m_rotationLocal = rotation;
        if (m_parent == entt::null) m_rotationWorld = rotation;
        m_dirty = true;
    }

    bool sgTransform::IsDirty() const
    {
        return m_dirty;
    }

    void sgTransform::SetParent(entt::entity newParent, const sgTransform* parentTransform)
    {
        m_parent = newParent;
        if (parentTransform != nullptr)
        {
            m_positionLocal = Vector3Subtract(m_positionWorld, parentTransform->m_positionWorld);
            m_rotationLocal = Vector3Subtract(m_rotationWorld, parentTransform->m_rotationWorld);
            const auto& ps = parentTransform->m_scaleWorld;
            m_scaleLocal = {
                ps.x != 0.0f ? m_scaleWorld.x / ps.x : m_scaleWorld.x,
                ps.y != 0.0f ? m_scaleWorld.y / ps.y : m_scaleWorld.y,
                ps.z != 0.0f ? m_scaleWorld.z / ps.z : m_scaleWorld.z};
        }
        else
        {
            m_positionLocal = m_positionWorld;
            m_rotationLocal = m_rotationWorld;
            m_scaleLocal = m_scaleWorld;
        }
        m_dirty = true;
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
        m_scaleWorld = rhs.m_scaleWorld;
        m_scaleLocal = rhs.m_scaleLocal;
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
    }

    sgTransform& sgTransform::operator=(const sgTransform& rhs)
    {
        m_positionLocal = rhs.m_positionLocal;
        m_rotationLocal = rhs.m_rotationLocal;
        m_positionWorld = rhs.m_positionWorld;
        m_rotationWorld = rhs.m_rotationWorld;
        m_scaleWorld = rhs.m_scaleWorld;
        m_scaleLocal = rhs.m_scaleLocal;
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
        m_scaleWorld = rhs.m_scaleWorld;
        m_scaleLocal = rhs.m_scaleLocal;
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
        m_scaleWorld = rhs.m_scaleWorld;
        m_scaleLocal = rhs.m_scaleLocal;
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
        return *this;
    }
} // namespace sage
