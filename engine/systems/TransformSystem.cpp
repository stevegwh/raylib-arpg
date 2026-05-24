#include "TransformSystem.hpp"

#include "components/sgTransform.hpp"
#include "raylib.h"
#include "slib.hpp"

namespace sage
{

    void TransformSystem::propagate(entt::entity entity, bool ancestorDirty)
    {
        auto& transform = registry->get<sgTransform>(entity);
        const bool effectivelyDirty = transform.dirty || ancestorDirty;

        if (effectivelyDirty && transform.m_parent != entt::null)
        {
            const auto& parent = registry->get<sgTransform>(transform.m_parent);
            transform.m_positionWorld = Vector3Add(parent.m_positionWorld, transform.m_positionLocal);
            transform.m_rotationWorld = Vector3Add(parent.m_rotationWorld, transform.m_rotationLocal);
            transform.m_scaleWorld = Vector3Multiply(parent.m_scaleWorld, transform.m_scaleLocal);
        }

        transform.dirty = false;

        for (auto child : transform.m_children)
        {
            propagate(child, effectivelyDirty);
        }
    }

    void TransformSystem::Update()
    {
        auto view = registry->view<sgTransform>();

        // Rebuild parent->children index from each transform's m_parent so SetParent
        // on the component alone is sufficient to reshape the hierarchy.
        for (auto entity : view)
        {
            view.get<sgTransform>(entity).m_children.clear();
        }
        for (auto entity : view)
        {
            const auto& transform = view.get<sgTransform>(entity);
            if (transform.m_parent != entt::null)
            {
                registry->get<sgTransform>(transform.m_parent).m_children.push_back(entity);
            }
        }

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
