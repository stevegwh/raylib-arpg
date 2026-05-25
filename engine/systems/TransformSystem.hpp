#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class sgTransform;

    class TransformSystem
    {
        entt::registry* registry;

        static Vector3 divideScale(const Vector3& worldScale, const Vector3& parentWorldScale);
        void addChild(entt::entity parent, entt::entity child) const;
        void removeChild(entt::entity parent, entt::entity child) const;
        void syncWorldFromLocal(entt::entity entity);
        void syncLocalFromWorld(entt::entity entity);
        void propagateChildren(entt::entity entity);
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        void SetWorldPos(entt::entity entity, const Vector3& position);
        void SetWorldRot(entt::entity entity, const Vector3& rotation);
        void SetWorldScale(entt::entity entity, const Vector3& scale);
        void SetWorldScale(entt::entity entity, float scale);
        void SetLocalPos(entt::entity entity, const Vector3& position);
        void SetLocalRot(entt::entity entity, const Vector3& rotation);
        void SetLocalRot(entt::entity entity, const Quaternion& rotation);
        void SetLocalScale(entt::entity entity, const Vector3& scale);
        void SetLocalScale(entt::entity entity, float scale);
        void SetParent(entt::entity entity, entt::entity newParent);

        explicit TransformSystem(entt::registry* _registry);
    };
} // namespace sage
