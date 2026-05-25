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
    class TransformSystem;

    class sgTransform
    {
        Vector3 m_positionWorld{};
        Vector3 m_positionLocal{};
        Vector3 m_rotationWorld{};
        Vector3 m_rotationLocal{};
        Vector3 m_scaleWorld{1, 1, 1};
        Vector3 m_scaleLocal{1, 1, 1};
        entt::entity m_parent = entt::null;
        std::vector<entt::entity> m_children{};

      public:
        Vector3 direction{};

        Ray movementDirectionDebugLine{};

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(
                m_positionWorld, m_rotationWorld, m_scaleWorld, m_positionLocal, m_rotationLocal, m_scaleLocal);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(
                m_positionWorld, m_rotationWorld, m_scaleWorld, m_positionLocal, m_rotationLocal, m_scaleLocal);
        }

        [[nodiscard]] Matrix GetMatrixNoRot() const;
        [[nodiscard]] Matrix GetMatrix() const;
        [[nodiscard]] Vector3 forward() const;
        [[nodiscard]] const Vector3& GetWorldPos() const;
        [[nodiscard]] const Vector3& GetLocalPos() const;
        [[nodiscard]] const Vector3& GetWorldRot() const;
        [[nodiscard]] const Vector3& GetLocalRot() const;
        [[nodiscard]] const Vector3& GetScale() const;
        [[nodiscard]] const Vector3& GetLocalScale() const;
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
