#include "TransformSystem.hpp"

#include "components/sgTransform.hpp"
#include "raylib.h"
#include "raymath.h"
#include "slib.hpp"

#include <algorithm>
#include <cassert>

namespace sage
{
    Vector3 TransformSystem::divideScale(const Vector3& worldScale, const Vector3& parentWorldScale)
    {
        return {
            parentWorldScale.x != 0.0f ? worldScale.x / parentWorldScale.x : worldScale.x,
            parentWorldScale.y != 0.0f ? worldScale.y / parentWorldScale.y : worldScale.y,
            parentWorldScale.z != 0.0f ? worldScale.z / parentWorldScale.z : worldScale.z};
    }

    void TransformSystem::addChild(entt::entity parent, entt::entity child) const
    {
        if (parent == entt::null) return;
        assert(registry->valid(parent));
        assert(registry->all_of<sgTransform>(parent));

        auto& children = registry->get<sgTransform>(parent).m_children;
        if (std::find(children.begin(), children.end(), child) == children.end())
        {
            children.push_back(child);
        }
    }

    void TransformSystem::removeChild(entt::entity parent, entt::entity child) const
    {
        if (parent == entt::null || !registry->valid(parent) || !registry->all_of<sgTransform>(parent)) return;

        auto& children = registry->get<sgTransform>(parent).m_children;
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }

    void TransformSystem::rebuildChildren()
    {
        auto view = registry->view<sgTransform>();

        for (auto entity : view)
        {
            view.get<sgTransform>(entity).m_children.clear();
        }

        for (auto entity : view)
        {
            const auto& transform = view.get<sgTransform>(entity);
            if (transform.m_parent != entt::null && registry->valid(transform.m_parent) &&
                registry->all_of<sgTransform>(transform.m_parent))
            {
                registry->get<sgTransform>(transform.m_parent).m_children.push_back(entity);
            }
        }
    }

    void TransformSystem::syncWorldFromLocal(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        if (transform.m_parent == entt::null)
        {
            transform.m_positionWorld = transform.m_positionLocal;
            transform.m_rotationWorld = transform.m_rotationLocal;
            transform.m_scaleWorld = transform.m_scaleLocal;
            return;
        }

        assert(registry->valid(transform.m_parent));
        assert(registry->all_of<sgTransform>(transform.m_parent));
        const auto& parent = registry->get<sgTransform>(transform.m_parent);
        transform.m_positionWorld = Vector3Add(parent.m_positionWorld, transform.m_positionLocal);
        transform.m_rotationWorld = Vector3Add(parent.m_rotationWorld, transform.m_rotationLocal);
        transform.m_scaleWorld = Vector3Multiply(parent.m_scaleWorld, transform.m_scaleLocal);
    }

    void TransformSystem::syncLocalFromWorld(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        if (transform.m_parent == entt::null)
        {
            transform.m_positionLocal = transform.m_positionWorld;
            transform.m_rotationLocal = transform.m_rotationWorld;
            transform.m_scaleLocal = transform.m_scaleWorld;
            return;
        }

        assert(registry->valid(transform.m_parent));
        assert(registry->all_of<sgTransform>(transform.m_parent));
        const auto& parent = registry->get<sgTransform>(transform.m_parent);
        transform.m_positionLocal = Vector3Subtract(transform.m_positionWorld, parent.m_positionWorld);
        transform.m_rotationLocal = Vector3Subtract(transform.m_rotationWorld, parent.m_rotationWorld);
        transform.m_scaleLocal = divideScale(transform.m_scaleWorld, parent.m_scaleWorld);
    }

    void TransformSystem::propagateChildren(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        for (auto child : transform.m_children)
        {
            if (!registry->valid(child) || !registry->all_of<sgTransform>(child)) continue;
            auto& childTransform = registry->get<sgTransform>(child);
            syncWorldFromLocal(child);
            propagateChildren(child);
        }
    }

    void TransformSystem::propagate(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        syncWorldFromLocal(entity);

        for (auto child : transform.m_children)
        {
            if (!registry->valid(child) || !registry->all_of<sgTransform>(child)) continue;
            propagate(child);
        }
    }

    void TransformSystem::SetWorldPos(entt::entity entity, const Vector3& position)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.m_positionWorld = position;
        syncLocalFromWorld(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetWorldRot(entt::entity entity, const Vector3& rotation)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.m_rotationWorld = rotation;
        syncLocalFromWorld(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetWorldScale(entt::entity entity, const Vector3& scale)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.m_scaleWorld = scale;
        syncLocalFromWorld(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetWorldScale(entt::entity entity, float scale)
    {
        SetWorldScale(entity, Vector3{scale, scale, scale});
    }

    void TransformSystem::SetLocalPos(entt::entity entity, const Vector3& position)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.m_positionLocal = position;
        syncWorldFromLocal(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetLocalRot(entt::entity entity, const Vector3& rotation)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.m_rotationLocal = rotation;
        syncWorldFromLocal(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetLocalRot(entt::entity entity, const Quaternion& rotation)
    {
        Vector3 rot = QuaternionToEuler(rotation);
        rot = Vector3MultiplyByValue(rot, RAD2DEG);
        SetLocalRot(entity, rot);
    }

    void TransformSystem::SetLocalScale(entt::entity entity, const Vector3& scale)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.m_scaleLocal = scale;
        syncWorldFromLocal(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetLocalScale(entt::entity entity, float scale)
    {
        SetLocalScale(entity, Vector3{scale, scale, scale});
    }

    void TransformSystem::SetParent(entt::entity entity, entt::entity newParent)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        assert(newParent != entity);

        if (newParent != entt::null)
        {
            assert(registry->valid(newParent));
            assert(registry->all_of<sgTransform>(newParent));
        }

        auto& transform = registry->get<sgTransform>(entity);
        if (transform.m_parent == newParent) return;

        removeChild(transform.m_parent, entity);
        transform.m_parent = newParent;
        addChild(newParent, entity);

        syncLocalFromWorld(entity);
        propagateChildren(entity);
    }

    void TransformSystem::Update()
    {
        rebuildChildren();

        auto view = registry->view<sgTransform>();
        for (auto entity : view)
        {
            const auto& transform = view.get<sgTransform>(entity);
            const bool hasValidParent = transform.m_parent != entt::null && registry->valid(transform.m_parent) &&
                                        registry->all_of<sgTransform>(transform.m_parent);
            if (!hasValidParent)
            {
                propagate(entity);
            }
        }
    }

    void TransformSystem::onComponentRemoved(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        removeChild(transform.m_parent, entity);

        const auto children = transform.m_children;
        for (auto child : children)
        {
            if (registry->valid(child) && registry->all_of<sgTransform>(child))
            {
                SetParent(child, entt::null);
            }
        }
        transform.m_children.clear();
    }

    void TransformSystem::onComponentAdded(entt::entity entity)
    {
        const auto& transform = registry->get<sgTransform>(entity);
        if (transform.m_parent != entt::null)
        {
            addChild(transform.m_parent, entity);
        }
    }

    TransformSystem::TransformSystem(entt::registry* _registry) : registry(_registry)
    {
        assert(registry != nullptr);
        registry->on_construct<sgTransform>().connect<&TransformSystem::onComponentAdded>(this);
        registry->on_destroy<sgTransform>().connect<&TransformSystem::onComponentRemoved>(this);
        rebuildChildren();
    }
} // namespace sage
