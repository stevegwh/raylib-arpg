//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

#include <Event.hpp>
#include <vector>

namespace sage
{
    class sgTransform
    {
        entt::entity self;
        Vector3 m_positionWorld{};
        Vector3 m_positionLocal{};
        Vector3 m_rotationWorld{};
        Vector3 m_rotationLocal{};
        Vector3 m_scale{};
        sgTransform* m_parent = nullptr;
        std::vector<sgTransform*> m_children;

        void updateChildrenPos();
        void updateChildrenRot();

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

        Event<entt::entity> onPositionUpdate{};

        [[nodiscard]] Matrix GetMatrixNoRot() const;
        [[nodiscard]] Matrix GetMatrix() const;
        [[nodiscard]] Vector3 forward() const;
        [[nodiscard]] const Vector3& GetWorldPos() const;
        [[nodiscard]] const Vector3& GetLocalPos() const;
        [[nodiscard]] const Vector3& GetWorldRot() const;
        [[nodiscard]] const Vector3& GetLocalRot() const;
        [[nodiscard]] const Vector3& GetScale() const;
        void SetLocalPos(const Vector3& position);
        void SetLocalRot(const Quaternion& rotation);
        void SetLocalRot(const Vector3& rotation);
        void SetPosition(const Vector3& position);
        void SetRotation(const Vector3& rotation);
        void SetScale(const Vector3& scale);
        void SetScale(float scale);
        void SetViaMatrix(Matrix mat);

        void SetParent(sgTransform* newParent);
        void AddChild(sgTransform* newChild);
        sgTransform* GetParent();
        const std::vector<sgTransform*>& GetChildren();

        explicit sgTransform(entt::entity _self);
        sgTransform(const sgTransform&) = delete;
        sgTransform& operator=(const sgTransform&) = delete;
    };
} // namespace sage
