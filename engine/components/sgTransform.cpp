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
    void sgTransform::SetParent(const entt::entity newParent)
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
        m_transformSystem->SetParent(m_entity, newParent);
    }

    void sgTransform::ResolveSerializedParent(
        const std::unordered_map<std::uint32_t, entt::entity>& idMap)
    {
        if (m_savedParentId == serializedNullId()) return;
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);

        const auto it = idMap.find(m_savedParentId);
        if (it != idMap.end())
        {
            m_transformSystem->SetParent(m_entity, it->second);
        }
        m_savedParentId = serializedNullId();
    }

    void sgTransform::writeLocalPos(const Vector3& v)
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
        m_transformSystem->SetLocalPos(m_entity, v);
    }

    void sgTransform::writeWorldPos(const Vector3& v)
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
        m_transformSystem->SetWorldPos(m_entity, v);
    }

    void sgTransform::writeLocalRot(const Vector3& v)
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
        m_transformSystem->SetLocalRot(m_entity, v);
    }

    void sgTransform::writeWorldRot(const Vector3& v)
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
        m_transformSystem->SetWorldRot(m_entity, v);
    }

    void sgTransform::writeLocalScale(const Vector3& v)
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
        m_transformSystem->SetLocalScale(m_entity, v);
    }

    void sgTransform::writeWorldScale(const Vector3& v)
    {
        assert(m_transformSystem != nullptr);
        assert(m_entity != entt::null);
        m_transformSystem->SetWorldScale(m_entity, v);
    }

    void sgTransform::Bind(TransformSystem* transformSystem, const entt::entity entity)
    {
        assert(transformSystem != nullptr);
        assert(entity != entt::null);
        m_transformSystem = transformSystem;
        m_entity = entity;
        rebindProxies();
    }

    void sgTransform::rebindProxies()
    {
        auto bindVec = [this](auto& field) {
            field.owner_ = this;
            field.x.parent = &field;
            field.y.parent = &field;
            field.z.parent = &field;
        };
        bindVec(position.local);
        bindVec(position.world);
        bindVec(rotation.local);
        bindVec(rotation.world);
        bindVec(scale.local);
        bindVec(scale.world);
    }

    Matrix sgTransform::GetMatrixNoRot() const
    {
        Matrix trans = MatrixTranslate(GetWorldPos().x, GetWorldPos().y, GetWorldPos().z);
        Matrix _scale = MatrixScale(GetScale().x, GetScale().y, GetScale().z);
        return MatrixMultiply(_scale, trans);
    }

    Matrix sgTransform::GetMatrix() const
    {
        Matrix trans = MatrixTranslate(GetWorldPos().x, GetWorldPos().y, GetWorldPos().z);
        Matrix _scale = MatrixScale(GetScale().x, GetScale().y, GetScale().z);
        Matrix rot = MatrixMultiply(
            MatrixMultiply(MatrixRotateZ(DEG2RAD * GetWorldRot().z), MatrixRotateY(DEG2RAD * GetWorldRot().y)),
            MatrixRotateX(DEG2RAD * GetWorldRot().x));
        return MatrixMultiply(MatrixMultiply(_scale, rot), trans);
    }

    Vector3 sgTransform::forward() const
    {
        Matrix matrix = GetMatrix();
        Vector3 forward = {matrix.m8, matrix.m9, matrix.m10};
        return Vector3Normalize(forward);
    }

    const Vector3& sgTransform::GetLocalPos() const
    {
        return position.local.value;
    }

    const Vector3& sgTransform::GetWorldPos() const
    {
        return position.world.value;
    }

    const Vector3& sgTransform::GetWorldRot() const
    {
        return rotation.world.value;
    }

    const Vector3& sgTransform::GetLocalRot() const
    {
        return rotation.local.value;
    }

    const Vector3& sgTransform::GetScale() const
    {
        return scale.world.value;
    }

    const Vector3& sgTransform::GetLocalScale() const
    {
        return scale.local.value;
    }

    entt::entity sgTransform::GetParent() const
    {
        return m_parent;
    }

    const std::vector<entt::entity>& sgTransform::GetChildren() const
    {
        return m_children;
    }

    sgTransform::sgTransform()
    {
        scale.local.value = {1, 1, 1};
        scale.world.value = {1, 1, 1};
        rebindProxies();
    }

    // Copy/move ctors and assignment operators transfer the full state
    // (proxy values + parent/children + binding + free fields) and rebind the
    // proxies' back-pointers to `this`. The on_construct signal in TransformSystem
    // will re-call Bind() when a copy is inserted into the registry via emplace,
    // overwriting the copied entity/system pointers with the correct values.
    sgTransform::sgTransform(const sgTransform& rhs)
        : m_entity(rhs.m_entity),
          m_transformSystem(rhs.m_transformSystem),
          m_parent(rhs.m_parent),
          m_children(rhs.m_children),
          direction(rhs.direction),
          movementDirectionDebugLine(rhs.movementDirectionDebugLine)
    {
        position.world.value = rhs.position.world.value;
        position.local.value = rhs.position.local.value;
        rotation.world.value = rhs.rotation.world.value;
        rotation.local.value = rhs.rotation.local.value;
        scale.world.value = rhs.scale.world.value;
        scale.local.value = rhs.scale.local.value;
        m_savedParentId = rhs.m_savedParentId;
        rebindProxies();
    }

    sgTransform& sgTransform::operator=(const sgTransform& rhs)
    {
        if (this == &rhs) return *this;
        m_entity = rhs.m_entity;
        m_transformSystem = rhs.m_transformSystem;
        m_parent = rhs.m_parent;
        m_children = rhs.m_children;
        direction = rhs.direction;
        movementDirectionDebugLine = rhs.movementDirectionDebugLine;
        position.world.value = rhs.position.world.value;
        position.local.value = rhs.position.local.value;
        rotation.world.value = rhs.rotation.world.value;
        rotation.local.value = rhs.rotation.local.value;
        scale.world.value = rhs.scale.world.value;
        scale.local.value = rhs.scale.local.value;
        m_savedParentId = rhs.m_savedParentId;
        return *this;
    }

    sgTransform::sgTransform(sgTransform&& rhs) noexcept
        : m_entity(rhs.m_entity),
          m_transformSystem(rhs.m_transformSystem),
          m_parent(rhs.m_parent),
          m_children(std::move(rhs.m_children)),
          direction(rhs.direction),
          movementDirectionDebugLine(rhs.movementDirectionDebugLine)
    {
        position.world.value = rhs.position.world.value;
        position.local.value = rhs.position.local.value;
        rotation.world.value = rhs.rotation.world.value;
        rotation.local.value = rhs.rotation.local.value;
        scale.world.value = rhs.scale.world.value;
        scale.local.value = rhs.scale.local.value;
        rhs.m_transformSystem = nullptr;
        rhs.m_entity = entt::null;
        rebindProxies();
    }

    sgTransform& sgTransform::operator=(sgTransform&& rhs) noexcept
    {
        if (this == &rhs) return *this;
        m_entity = rhs.m_entity;
        m_transformSystem = rhs.m_transformSystem;
        m_parent = rhs.m_parent;
        m_children = std::move(rhs.m_children);
        direction = rhs.direction;
        movementDirectionDebugLine = rhs.movementDirectionDebugLine;
        position.world.value = rhs.position.world.value;
        position.local.value = rhs.position.local.value;
        rotation.world.value = rhs.rotation.world.value;
        rotation.local.value = rhs.rotation.local.value;
        scale.world.value = rhs.scale.world.value;
        scale.local.value = rhs.scale.local.value;
        rhs.m_transformSystem = nullptr;
        rhs.m_entity = entt::null;
        return *this;
    }
} // namespace sage
