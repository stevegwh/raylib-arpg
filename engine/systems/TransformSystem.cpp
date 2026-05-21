#include "TransformSystem.hpp"

#include "components/sgTransform.hpp"
#include "raylib.h"
#include "slib.hpp"

namespace sage
{

    void TransformSystem::SetLocalPos(entt::entity entity, const Vector3& position)
    {
        registry->get<sgTransform>(entity).SetLocalPos(position);
    }

    void TransformSystem::SetLocalRot(entt::entity entity, const Quaternion& rotation)
    {
        Vector3 rot = QuaternionToEuler(rotation);
        rot = Vector3MultiplyByValue(rot, RAD2DEG);
        SetLocalRot(entity, rot);
    }

    void TransformSystem::SetLocalRot(entt::entity entity, const Vector3& rotation)
    {
        registry->get<sgTransform>(entity).SetLocalRot(rotation);
    }

    void TransformSystem::SetPosition(entt::entity entity, const Vector3& position)
    {
        registry->get<sgTransform>(entity).SetWorldPos(position);
    }

    void TransformSystem::SetRotation(entt::entity entity, const Vector3& rotation)
    {
        registry->get<sgTransform>(entity).SetWorldRot(rotation);
    }

    void TransformSystem::SetScale(entt::entity entity, const Vector3& scale)
    {
        registry->get<sgTransform>(entity).SetWorldScale(scale);
    }

    void TransformSystem::SetScale(entt::entity entity, float scale)
    {
        registry->get<sgTransform>(entity).SetWorldScale({scale, scale, scale});
    }

    void TransformSystem::SetParent(entt::entity entity, entt::entity newParent) const
    {
        auto& transform = registry->get<sgTransform>(entity);

        if (transform.m_parent != entt::null)
        {
            auto& parentChildren = registry->get<sgTransform>(transform.m_parent).m_children;
            auto it = std::find(parentChildren.begin(), parentChildren.end(), entity);
            if (it != parentChildren.end()) parentChildren.erase(it);
        }

        transform.m_parent = newParent;

        if (transform.m_parent != entt::null)
        {
            auto& parent = registry->get<sgTransform>(transform.m_parent);
            parent.m_children.push_back(entity);
            transform.m_positionLocal = Vector3Subtract(transform.m_positionWorld, parent.GetWorldPos());
            transform.m_rotationLocal = Vector3Subtract(transform.m_rotationWorld, parent.GetWorldRot());
        }
        else
        {
            transform.m_positionLocal = transform.m_positionWorld;
            transform.m_rotationLocal = transform.m_rotationWorld;
        }
        transform.m_dirty = true;
    }

    void TransformSystem::AddChild(entt::entity entity, entt::entity newChild) const
    {
        SetParent(newChild, entity);
    }

    void TransformSystem::SetViaMatrix(entt::entity entity, Matrix mat)
    {
        Matrix newMat{};
        Vector3 trans{};
        Quaternion rotQ{};
        Vector3 scale{};
        MatrixDecompose(newMat, &trans, &rotQ, &scale);
        Vector3 rot = QuaternionToEuler(rotQ);
        auto& transform = registry->get<sgTransform>(entity);
        transform.SetWorldScale(scale);
        transform.SetWorldRot(rot);
        transform.SetWorldPos(trans);
    }

    void TransformSystem::propagate(entt::entity entity, bool ancestorDirty)
    {
        auto& transform = registry->get<sgTransform>(entity);
        const bool effectivelyDirty = transform.m_dirty || ancestorDirty;

        if (effectivelyDirty && transform.m_parent != entt::null)
        {
            const auto& parent = registry->get<sgTransform>(transform.m_parent);
            transform.m_positionWorld = Vector3Add(parent.m_positionWorld, transform.m_positionLocal);
            transform.m_rotationWorld = Vector3Add(parent.m_rotationWorld, transform.m_rotationLocal);
        }

        transform.m_dirty = false;

        for (auto child : transform.m_children)
        {
            propagate(child, effectivelyDirty);
        }
    }

    void TransformSystem::Update()
    {
        auto view = registry->view<sgTransform>();
        for (auto entity : view)
        {
            const auto& transform = view.get<sgTransform>(entity);
            if (transform.m_parent == entt::null)
            {
                propagate(entity, false);
            }
        }
    }

    void TransformSystem::onComponentRemoved(entt::entity entity)
    {
    }

    void TransformSystem::onComponentAdded(entt::entity entity)
    {
    }

    TransformSystem::TransformSystem(entt::registry* _registry) : registry(_registry)
    {
        registry->on_construct<sgTransform>().connect<&TransformSystem::onComponentAdded>(this);
        registry->on_destroy<sgTransform>().connect<&TransformSystem::onComponentRemoved>(this);
    }
} // namespace sage
