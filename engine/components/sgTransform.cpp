//
// Created by Steve Wheeler on 03/05/2024.
//

#include "sgTransform.hpp"
#include "raymath.h"
#include "slib.hpp"

namespace sage
{
    sgTransform::sgTransform()
        : m_positionWorld(*this), m_positionLocal(*this), m_rotationWorld(*this), m_rotationLocal(*this),
          m_scaleWorld(*this, Vector3{1, 1, 1}), m_scaleLocal(*this, Vector3{1, 1, 1})
    {
    }

    Matrix sgTransform::GetMatrixNoRot() const
    {
        Matrix trans = MatrixTranslate(GetWorldPos().x, GetWorldPos().y, GetWorldPos().z);
        Matrix _scale = MatrixScale(GetScale().x, GetScale().y, GetScale().z);
        return MatrixMultiply(trans, _scale);
    }

    Matrix sgTransform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(GetWorldPos().x, GetWorldPos().y, GetWorldPos().z);
        Matrix _scale = MatrixScale(GetScale().x, GetScale().y, GetScale().z);
        Matrix rot = MatrixMultiply(
            MatrixMultiply(MatrixRotateZ(DEG2RAD * GetWorldRot().z), MatrixRotateY(DEG2RAD * GetWorldRot().y)),
            MatrixRotateX(DEG2RAD * GetWorldRot().x));
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
        return *m_positionLocal;
    }

    const Vector3& sgTransform::GetWorldPos() const
    {
        return *m_positionWorld;
    }

    const Vector3& sgTransform::GetWorldRot() const
    {
        return *m_rotationWorld;
    }

    const Vector3& sgTransform::GetLocalRot() const
    {
        return *m_rotationLocal;
    }

    const Vector3& sgTransform::GetScale() const
    {
        return *m_scaleWorld;
    }

    const Vector3& sgTransform::GetLocalScale() const
    {
        return *m_scaleLocal;
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
        m_scaleWorld = scale;
        if (m_parent == entt::null) m_scaleLocal = scale;
    }

    void sgTransform::SetWorldScale(float scale)
    {
        SetWorldScale(Vector3{scale, scale, scale});
    }

    void sgTransform::SetLocalScale(const Vector3& scale)
    {
        m_scaleLocal = scale;
        if (m_parent == entt::null) m_scaleWorld = scale;
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
    }

    void sgTransform::SetLocalRot(const Vector3& rotation)
    {
        m_rotationLocal = rotation;
        if (m_parent == entt::null) m_rotationWorld = rotation;
    }

    bool sgTransform::IsDirty() const
    {
        return dirty;
    }

    void sgTransform::SetParent(entt::entity newParent, const sgTransform* parentTransform)
    {
        m_parent = newParent;
        if (parentTransform != nullptr)
        {
            m_positionLocal = Vector3Subtract(*m_positionWorld, *parentTransform->m_positionWorld);
            m_rotationLocal = Vector3Subtract(*m_rotationWorld, *parentTransform->m_rotationWorld);
            const auto& ps = *parentTransform->m_scaleWorld;
            m_scaleLocal = {
                ps.x != 0.0f ? m_scaleWorld.value.x / ps.x : m_scaleWorld.value.x,
                ps.y != 0.0f ? m_scaleWorld.value.y / ps.y : m_scaleWorld.value.y,
                ps.z != 0.0f ? m_scaleWorld.value.z / ps.z : m_scaleWorld.value.z};
        }
        else
        {
            m_positionLocal = m_positionWorld;
            m_rotationLocal = m_rotationWorld;
            m_scaleLocal = m_scaleWorld;
        }
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
        : Component(rhs), m_positionWorld(*this, rhs.m_positionWorld), m_positionLocal(*this, rhs.m_positionLocal),
          m_rotationWorld(*this, rhs.m_rotationWorld),
          m_rotationLocal(*this, rhs.m_rotationLocal), m_scaleWorld(*this, rhs.m_scaleWorld),
          m_scaleLocal(*this, rhs.m_scaleLocal)
    {
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
    }

    sgTransform& sgTransform::operator=(const sgTransform& rhs)
    {
        if (this == &rhs) return *this;
        Component<sgTransform>::operator=(rhs);
        m_positionLocal.value = rhs.m_positionLocal.value;
        m_rotationLocal.value = rhs.m_rotationLocal.value;
        m_positionWorld.value = rhs.m_positionWorld.value;
        m_rotationWorld.value = rhs.m_rotationWorld.value;
        m_scaleWorld.value = rhs.m_scaleWorld.value;
        m_scaleLocal.value = rhs.m_scaleLocal.value;
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
        return *this; // ADDED: missing return statement
    }

    sgTransform::sgTransform(sgTransform&& rhs) noexcept
        : Component(rhs), m_positionWorld(*this, std::move(rhs.m_positionWorld)),
          m_positionLocal(*this, std::move(rhs.m_positionLocal)),
          m_rotationWorld(*this, std::move(rhs.m_rotationWorld)),
          m_rotationLocal(*this, std::move(rhs.m_rotationLocal)),
          m_scaleWorld(*this, std::move(rhs.m_scaleWorld)), m_scaleLocal(*this, std::move(rhs.m_scaleLocal))
    {
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
    }

    // ADDED: Missing move assignment operator
    sgTransform& sgTransform::operator=(sgTransform&& rhs) noexcept
    {
        if (this == &rhs) return *this;
        Component<sgTransform>::operator=(rhs);
        m_positionLocal.value = std::move(rhs.m_positionLocal.value);
        m_rotationLocal.value = std::move(rhs.m_rotationLocal.value);
        m_positionWorld.value = std::move(rhs.m_positionWorld.value);
        m_rotationWorld.value = std::move(rhs.m_rotationWorld.value);
        m_scaleWorld.value = std::move(rhs.m_scaleWorld.value);
        m_scaleLocal.value = std::move(rhs.m_scaleLocal.value);
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
        return *this;
    }
} // namespace sage
