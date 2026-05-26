//
// Created by Steve Wheeler on 03/05/2024.
//

#include "sgTransform.hpp"
#include "raymath.h"
#include "slib.hpp"
#include "systems/TransformSystem.hpp"

#include <cassert>
#include <utility>

namespace sage
{
    sgTransform::TransformAxisAccessor::TransformAxisAccessor(
        sgTransform* owner, const TransformReader read, const TransformWriter write, const VectorAxis axis)
        : owner(owner), read(read), write(write), axis(axis)
    {
    }

    void sgTransform::TransformAxisAccessor::BindOwner(sgTransform* newOwner)
    {
        owner = newOwner;
    }

    sgTransform::TransformAxisAccessor& sgTransform::TransformAxisAccessor::operator=(const float value)
    {
        assert(owner != nullptr);
        assert(read != nullptr);
        assert(write != nullptr);
        assert(axis != nullptr);

        auto vector = (owner->*read)();
        vector.*axis = value;
        (owner->*write)(vector);
        return *this;
    }

    sgTransform::TransformAxisAccessor& sgTransform::TransformAxisAccessor::operator=(
        const TransformAxisAccessor& rhs)
    {
        return *this = static_cast<float>(rhs);
    }

    sgTransform::TransformAxisAccessor::operator float() const
    {
        assert(owner != nullptr);
        assert(read != nullptr);
        assert(axis != nullptr);

        const auto& value = (owner->*read)();
        return value.*axis;
    }

    sgTransform::TransformVectorAccessor::TransformVectorAccessor(
        sgTransform* owner, const TransformReader read, const TransformWriter write)
        : owner(owner),
          read(read),
          write(write),
          x(owner, read, write, &Vector3::x),
          y(owner, read, write, &Vector3::y),
          z(owner, read, write, &Vector3::z)
    {
    }

    void sgTransform::TransformVectorAccessor::BindOwner(sgTransform* newOwner)
    {
        owner = newOwner;
        x.BindOwner(newOwner);
        y.BindOwner(newOwner);
        z.BindOwner(newOwner);
    }

    sgTransform::TransformVectorAccessor& sgTransform::TransformVectorAccessor::operator=(const Vector3& value)
    {
        assert(owner != nullptr);
        assert(write != nullptr);
        (owner->*write)(value);
        return *this;
    }

    sgTransform::TransformVectorAccessor& sgTransform::TransformVectorAccessor::operator=(
        const TransformVectorAccessor& rhs)
    {
        return *this = rhs.Get();
    }

    sgTransform::TransformVectorAccessor::operator Vector3() const
    {
        return Get();
    }

    const Vector3& sgTransform::TransformVectorAccessor::Get() const
    {
        assert(owner != nullptr);
        assert(read != nullptr);
        return (owner->*read)();
    }

    sgTransform::TransformAccessorGroup::TransformAccessorGroup(
        sgTransform* owner,
        const TransformReader localRead,
        const TransformWriter localWrite,
        const TransformReader worldRead,
        const TransformWriter worldWrite)
        : local(owner, localRead, localWrite), world(owner, worldRead, worldWrite)
    {
    }

    void sgTransform::TransformAccessorGroup::BindOwner(sgTransform* newOwner)
    {
        local.BindOwner(newOwner);
        world.BindOwner(newOwner);
    }

    void sgTransform::AssertBound() const
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
    }

    void sgTransform::SetLocalPosViaSystem(const Vector3& position)
    {
        AssertBound();
        m_transformSystem->SetLocalPos(m_entity, position);
    }

    void sgTransform::SetWorldPosViaSystem(const Vector3& position)
    {
        AssertBound();
        m_transformSystem->SetWorldPos(m_entity, position);
    }

    void sgTransform::SetLocalRotViaSystem(const Vector3& rotation)
    {
        AssertBound();
        m_transformSystem->SetLocalRot(m_entity, rotation);
    }

    void sgTransform::SetWorldRotViaSystem(const Vector3& rotation)
    {
        AssertBound();
        m_transformSystem->SetWorldRot(m_entity, rotation);
    }

    void sgTransform::SetLocalScaleViaSystem(const Vector3& scale)
    {
        AssertBound();
        m_transformSystem->SetLocalScale(m_entity, scale);
    }

    void sgTransform::SetWorldScaleViaSystem(const Vector3& scale)
    {
        AssertBound();
        m_transformSystem->SetWorldScale(m_entity, scale);
    }

    void sgTransform::Bind(TransformSystem* transformSystem, const entt::entity entity)
    {
        assert(transformSystem != nullptr);
        assert(entity != entt::null);
        m_transformSystem = transformSystem;
        m_entity = entity;
        position.BindOwner(this);
        rotation.BindOwner(this);
        scale.BindOwner(this);
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
          m_scaleLocal(rhs.m_scaleLocal),
          position(
              this,
              &sgTransform::GetLocalPos,
              &sgTransform::SetLocalPosViaSystem,
              &sgTransform::GetWorldPos,
              &sgTransform::SetWorldPosViaSystem),
          rotation(
              this,
              &sgTransform::GetLocalRot,
              &sgTransform::SetLocalRotViaSystem,
              &sgTransform::GetWorldRot,
              &sgTransform::SetWorldRotViaSystem),
          scale(
              this,
              &sgTransform::GetLocalScale,
              &sgTransform::SetLocalScaleViaSystem,
              &sgTransform::GetScale,
              &sgTransform::SetWorldScaleViaSystem)
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
        position.BindOwner(this);
        rotation.BindOwner(this);
        scale.BindOwner(this);
        return *this;
    }

    sgTransform::sgTransform(sgTransform&& rhs) noexcept
        : m_positionWorld(rhs.m_positionWorld),
          m_positionLocal(rhs.m_positionLocal),
          m_rotationWorld(rhs.m_rotationWorld),
          m_rotationLocal(rhs.m_rotationLocal),
          m_scaleWorld(rhs.m_scaleWorld),
          m_scaleLocal(rhs.m_scaleLocal),
          m_entity(rhs.m_entity),
          m_transformSystem(rhs.m_transformSystem),
          position(
              this,
              &sgTransform::GetLocalPos,
              &sgTransform::SetLocalPosViaSystem,
              &sgTransform::GetWorldPos,
              &sgTransform::SetWorldPosViaSystem),
          rotation(
              this,
              &sgTransform::GetLocalRot,
              &sgTransform::SetLocalRotViaSystem,
              &sgTransform::GetWorldRot,
              &sgTransform::SetWorldRotViaSystem),
          scale(
              this,
              &sgTransform::GetLocalScale,
              &sgTransform::SetLocalScaleViaSystem,
              &sgTransform::GetScale,
              &sgTransform::SetWorldScaleViaSystem)
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
        if (m_transformSystem == nullptr && m_entity == entt::null)
        {
            m_transformSystem = rhs.m_transformSystem;
            m_entity = rhs.m_entity;
        }
        position.BindOwner(this);
        rotation.BindOwner(this);
        scale.BindOwner(this);
        return *this;
    }

    sgTransform::sgTransform()
        : position(
              this,
              &sgTransform::GetLocalPos,
              &sgTransform::SetLocalPosViaSystem,
              &sgTransform::GetWorldPos,
              &sgTransform::SetWorldPosViaSystem),
          rotation(
              this,
              &sgTransform::GetLocalRot,
              &sgTransform::SetLocalRotViaSystem,
              &sgTransform::GetWorldRot,
              &sgTransform::SetWorldRotViaSystem),
          scale(
              this,
              &sgTransform::GetLocalScale,
              &sgTransform::SetLocalScaleViaSystem,
              &sgTransform::GetScale,
              &sgTransform::SetWorldScaleViaSystem)
    {
    }
} // namespace sage
