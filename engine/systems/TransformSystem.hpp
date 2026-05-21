#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class TransformSystem
    {
        entt::registry* registry;

        void propagate(entt::entity entity, bool ancestorDirty);
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        void SetLocalPos(entt::entity entity, const Vector3& position);
        void SetLocalRot(entt::entity entity, const Quaternion& rotation);
        void SetLocalRot(entt::entity entity, const Vector3& rotation);
        void SetPosition(entt::entity entity, const Vector3& position);
        void SetRotation(entt::entity entity, const Vector3& rotation);
        void SetScale(entt::entity entity, const Vector3& scale);
        void SetScale(entt::entity entity, float scale);
        void SetViaMatrix(entt::entity entity, Matrix mat);
        void SetParent(entt::entity entity, entt::entity newParent) const;
        void AddChild(entt::entity entity, entt::entity newChild) const;
        void Update();

        explicit TransformSystem(entt::registry* _registry);
    };
} // namespace sage