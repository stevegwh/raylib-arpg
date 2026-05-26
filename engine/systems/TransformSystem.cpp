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

    void TransformSystem::syncWorldFromLocal(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        if (transform.m_parent == entt::null)
        {
            transform.position.world.value = transform.position.local.value;
            transform.rotation.world.value = transform.rotation.local.value;
            transform.scale.world.value = transform.scale.local.value;
            return;
        }

        assert(registry->valid(transform.m_parent));
        assert(registry->all_of<sgTransform>(transform.m_parent));
        const auto& parentTransform = registry->get<sgTransform>(transform.m_parent);
        transform.position.world.value =
            Vector3Add(parentTransform.position.world.value, transform.position.local.value);
        transform.rotation.world.value =
            Vector3Add(parentTransform.rotation.world.value, transform.rotation.local.value);
        transform.scale.world.value =
            Vector3Multiply(parentTransform.scale.world.value, transform.scale.local.value);
    }

    void TransformSystem::syncLocalFromWorld(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        if (transform.m_parent == entt::null)
        {
            transform.position.local.value = transform.position.world.value;
            transform.rotation.local.value = transform.rotation.world.value;
            transform.scale.local.value = transform.scale.world.value;
            return;
        }

        assert(registry->valid(transform.m_parent));
        assert(registry->all_of<sgTransform>(transform.m_parent));
        const auto& parentTransform = registry->get<sgTransform>(transform.m_parent);
        transform.position.local.value =
            Vector3Subtract(transform.position.world.value, parentTransform.position.world.value);
        transform.rotation.local.value =
            Vector3Subtract(transform.rotation.world.value, parentTransform.rotation.world.value);
        transform.scale.local.value = divideScale(transform.scale.world.value, parentTransform.scale.world.value);
    }

    void TransformSystem::propagateChildren(entt::entity entity)
    {
        auto& transform = registry->get<sgTransform>(entity);
        for (auto child : transform.m_children)
        {
            if (!registry->valid(child) || !registry->all_of<sgTransform>(child)) continue;
            syncWorldFromLocal(child);
            propagateChildren(child);
        }
    }

    void TransformSystem::bindExistingTransforms()
    {
        for (const auto entity : registry->view<sgTransform>())
        {
            onComponentAdded(entity);
        }
    }

    void TransformSystem::SetWorldPos(entt::entity entity, const Vector3& position)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.position.world.value = position;
        syncLocalFromWorld(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetWorldRot(entt::entity entity, const Vector3& rotation)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.rotation.world.value = rotation;
        syncLocalFromWorld(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetWorldScale(entt::entity entity, const Vector3& scale)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.scale.world.value = scale;
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
        transform.position.local.value = position;
        syncWorldFromLocal(entity);
        propagateChildren(entity);
    }

    void TransformSystem::SetLocalRot(entt::entity entity, const Vector3& rotation)
    {
        assert(registry->valid(entity));
        assert(registry->all_of<sgTransform>(entity));
        auto& transform = registry->get<sgTransform>(entity);
        transform.rotation.local.value = rotation;
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
        transform.scale.local.value = scale;
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
        auto& transform = registry->get<sgTransform>(entity);
        transform.Bind(this, entity);
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
        bindExistingTransforms();
    }
} // namespace sage
