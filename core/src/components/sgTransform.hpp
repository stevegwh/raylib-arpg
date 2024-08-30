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
        entt::entity self;
        Vector3 m_positionWorld{};
        Vector3 m_positionLocal{};
        Vector3 m_rotation{};
        float m_scale = 1.0f;
        sgTransform* m_parent = nullptr;
        std::vector<sgTransform*> m_children;

        void updateChildren();

      public:
        Vector3 direction{};
        float movementSpeed = 0.35f;

        Ray movementDirectionDebugLine{};

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(m_positionWorld, m_rotation, m_scale);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(m_positionWorld, m_rotation, m_scale);
        }

        entt::sigh<void(entt::entity)> onPositionUpdate{};

        [[nodiscard]] Matrix GetMatrixNoRot() const;
        [[nodiscard]] Matrix GetMatrix() const;
        [[nodiscard]] Vector3 forward() const;
        [[nodiscard]] const Vector3& GetWorldPos() const;
        [[nodiscard]] const Vector3& GetLocalPos() const;
        [[nodiscard]] const Vector3& GetRotation() const;
        [[nodiscard]] float GetScale() const;
        void SetPosition(const Vector3& position);
        void SetRotation(const Vector3& rotation);
        void SetScale(float scale);
        sgTransform* GetParent();
        const std::vector<sgTransform*>& GetChildren();
        void SetParent(sgTransform* newParent);
        void AddChild(sgTransform* newChild);

        // sgTransform() = default;
        sgTransform(entt::entity _self);
        sgTransform(const sgTransform&) = delete;
        sgTransform& operator=(const sgTransform&) = delete;
    };
} // namespace sage
