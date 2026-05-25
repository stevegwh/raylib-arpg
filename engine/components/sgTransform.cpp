//
// Created by Steve Wheeler on 03/05/2024.
//

#include "sgTransform.hpp"
#include "raymath.h"
#include "slib.hpp"

#include <utility>

namespace sage
{

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

    entt::entity sgTransform::GetParent() const
    {
        return m_parent;
    }

    const std::vector<entt::entity>& sgTransform::GetChildren() const
    {
        return m_children;
    }

    sgTransform::sgTransform(const sgTransform& rhs)
        : m_positionWorld(rhs.m_positionWorld),
          m_positionLocal(rhs.m_positionLocal),
          m_rotationWorld(rhs.m_rotationWorld),
          m_rotationLocal(rhs.m_rotationLocal),
          m_scaleWorld(rhs.m_scaleWorld),
          m_scaleLocal(rhs.m_scaleLocal)
    {
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
    }

    sgTransform& sgTransform::operator=(const sgTransform& rhs)
    {
        if (this == &rhs) return *this;
        m_positionLocal = rhs.m_positionLocal;
        m_rotationLocal = rhs.m_rotationLocal;
        m_positionWorld = rhs.m_positionWorld;
        m_rotationWorld = rhs.m_rotationWorld;
        m_scaleWorld = rhs.m_scaleWorld;
        m_scaleLocal = rhs.m_scaleLocal;
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
        return *this;
    }

    sgTransform::sgTransform(sgTransform&& rhs) noexcept
        : m_positionWorld(rhs.m_positionWorld),
          m_positionLocal(rhs.m_positionLocal),
          m_rotationWorld(rhs.m_rotationWorld),
          m_rotationLocal(rhs.m_rotationLocal),
          m_scaleWorld(rhs.m_scaleWorld),
          m_scaleLocal(rhs.m_scaleLocal)
    {
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
    }

    sgTransform& sgTransform::operator=(sgTransform&& rhs) noexcept
    {
        if (this == &rhs) return *this;
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
