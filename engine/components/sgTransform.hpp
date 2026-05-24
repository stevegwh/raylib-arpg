//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "Component.hpp"
#include "ComponentField.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    class sgTransform : public Component<sgTransform>
    {
        ComponentField<Vector3, sgTransform> m_positionWorld;
        ComponentField<Vector3, sgTransform> m_positionLocal;
        ComponentField<Vector3, sgTransform> m_rotationWorld;
        ComponentField<Vector3, sgTransform> m_rotationLocal;
        ComponentField<Vector3, sgTransform> m_scaleWorld;
        ComponentField<Vector3, sgTransform> m_scaleLocal;
        entt::entity m_parent = entt::null;
        std::vector<entt::entity> m_children;

      public:
        Vector3 direction{};

        Ray movementDirectionDebugLine{};

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(
                m_positionWorld.value,
                m_rotationWorld.value,
                m_scaleWorld.value,
                m_positionLocal.value,
                m_rotationLocal.value,
                m_scaleLocal.value);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(
                m_positionWorld.value,
                m_rotationWorld.value,
                m_scaleWorld.value,
                m_positionLocal.value,
                m_rotationLocal.value,
                m_scaleLocal.value);
        }

        template <class Inspector>
        void define_editor_fields(Inspector& i)
        {
            i.field("Position", m_positionLocal, [this](const Vector3& position) { SetLocalPos(position); });
            i.field("Rotation", m_rotationLocal, [this](const Vector3& rotation) { SetLocalRot(rotation); });
            i.field("Scale", m_scaleLocal, [this](const Vector3& scale) { SetLocalScale(scale); });
        }

        [[nodiscard]] Matrix GetMatrixNoRot() const;
        [[nodiscard]] Matrix GetMatrix() const;
        [[nodiscard]] Vector3 forward() const;
        [[nodiscard]] const Vector3& GetWorldPos() const;
        [[nodiscard]] const Vector3& GetLocalPos() const;
        [[nodiscard]] const Vector3& GetWorldRot() const;
        [[nodiscard]] const Vector3& GetLocalRot() const;
        [[nodiscard]] const Vector3& GetScale() const;
        void SetWorldPos(const Vector3& position);
        void SetWorldRot(const Vector3& rotation);
        void SetWorldScale(const Vector3& scale);
        void SetWorldScale(float scale);
        void SetLocalPos(const Vector3& position);
        void SetLocalRot(const Vector3& rotation);
        void SetLocalRot(const Quaternion& rotation);
        void SetLocalScale(const Vector3& scale);
        void SetLocalScale(float scale);
        [[nodiscard]] const Vector3& GetLocalScale() const;
        void SetParent(entt::entity newParent, const sgTransform* parentTransform);
        [[nodiscard]] bool IsDirty() const;
        entt::entity GetParent() const;
        const std::vector<entt::entity>& GetChildren() const;

        sgTransform(const sgTransform& rhs);
        sgTransform& operator=(const sgTransform& rhs);
        sgTransform(sgTransform&& rhs) noexcept;
        sgTransform& operator=(sgTransform&& rhs) noexcept;
        sgTransform();

        friend class TransformSystem;
    };
} // namespace sage
