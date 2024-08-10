//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class sgTransform
    {
        Vector3 m_position{};
        Vector3 m_rotation{};
        float m_scale = 1.0f;

      public:
        Vector3 direction{};
        float movementSpeed = 0.35f;

        Ray movementDirectionDebugLine{};

        sgTransform() = default;
        sgTransform(const sgTransform&) = delete;
        sgTransform& operator=(const sgTransform&) = delete;

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(m_position, m_rotation, m_scale);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(m_position, m_rotation, m_scale);
        }

        entt::sigh<void(entt::entity)> onPositionUpdate{};

        [[nodiscard]] Matrix GetMatrixNoRot() const;
        [[nodiscard]] Matrix GetMatrix() const;
        [[nodiscard]] Vector3 forward() const;
        [[nodiscard]] const Vector3& position() const;
        [[nodiscard]] const Vector3& rotation() const;
        [[nodiscard]] float scale() const;
        void SetPosition(const Vector3& position, const entt::entity& entity);
        void SetRotation(const Vector3& rotation, const entt::entity& entity);
        void SetScale(float scale, const entt::entity& entity);
    };
} // namespace sage
