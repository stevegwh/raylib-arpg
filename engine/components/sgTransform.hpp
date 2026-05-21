//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

#include <vector>

namespace sage
{
    class sgTransform
    {
        Vector3 m_positionWorld{};
        Vector3 m_positionLocal{};
        Vector3 m_rotationWorld{};
        Vector3 m_rotationLocal{};
        Vector3 m_scale{1, 1, 1};
        entt::entity m_parent = entt::null;
        std::vector<entt::entity> m_children;
        bool m_dirty = true;

      public:
        Vector3 direction{};

        Ray movementDirectionDebugLine{};

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(m_positionWorld, m_rotationWorld, m_scale);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(m_positionWorld, m_rotationWorld, m_scale);
        }

        template <class Inspector>
        void define_editor_fields(Inspector& i)
        {
            i.field("Position", m_positionWorld, [this](const Vector3& position) { SetWorldPos(position); });
            i.field("Rotation", m_rotationWorld, [this](const Vector3& rotation) { SetWorldRot(rotation); });
            i.field("Scale", m_scale, [this](const Vector3& scale) { SetWorldScale(scale); });
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
        void SetLocalPos(const Vector3& position);
        void SetLocalRot(const Vector3& rotation);
        [[nodiscard]] bool IsDirty() const;
        entt::entity GetParent() const;
        const std::vector<entt::entity>& GetChildren() const;

        sgTransform(const sgTransform& rhs);
        sgTransform& operator=(const sgTransform& rhs);
        sgTransform(sgTransform&& rhs) noexcept;
        sgTransform& operator=(sgTransform&& rhs) noexcept;
        sgTransform() = default;

        friend class TransformSystem;
    };
} // namespace sage
