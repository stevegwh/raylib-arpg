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
        entt::entity m_entity = entt::null;
        TransformSystem* m_transformSystem = nullptr;

        using TransformReader = const Vector3& (sgTransform::*)() const;
        using TransformWriter = void (sgTransform::*)(const Vector3&);
        using VectorAxis = float Vector3::*;

        void AssertBound() const;
        void SetLocalPosViaSystem(const Vector3& position);
        void SetWorldPosViaSystem(const Vector3& position);
        void SetLocalRotViaSystem(const Vector3& rotation);
        void SetWorldRotViaSystem(const Vector3& rotation);
        void SetLocalScaleViaSystem(const Vector3& scale);
        void SetWorldScaleViaSystem(const Vector3& scale);
        void Bind(TransformSystem* transformSystem, entt::entity entity);

      public:
        class TransformAxisAccessor
        {
            sgTransform* owner = nullptr;
            TransformReader read = nullptr;
            TransformWriter write = nullptr;
            VectorAxis axis = nullptr;

            TransformAxisAccessor(
                sgTransform* owner, TransformReader read, TransformWriter write, VectorAxis axis);
            void BindOwner(sgTransform* newOwner);

            friend class sgTransform;
            friend class TransformVectorAccessor;

          public:
            TransformAxisAccessor() = default;
            TransformAxisAccessor& operator=(float value);
            TransformAxisAccessor& operator=(const TransformAxisAccessor& rhs);
            [[nodiscard]] operator float() const;
        };

        class TransformVectorAccessor
        {
            sgTransform* owner = nullptr;
            TransformReader read = nullptr;
            TransformWriter write = nullptr;

            TransformVectorAccessor(sgTransform* owner, TransformReader read, TransformWriter write);
            void BindOwner(sgTransform* newOwner);

            friend class sgTransform;
            friend class TransformAccessorGroup;

          public:
            TransformAxisAccessor x;
            TransformAxisAccessor y;
            TransformAxisAccessor z;

            TransformVectorAccessor() = default;
            TransformVectorAccessor& operator=(const Vector3& value);
            TransformVectorAccessor& operator=(const TransformVectorAccessor& rhs);
            [[nodiscard]] operator Vector3() const;
            [[nodiscard]] const Vector3& Get() const;
        };

        class TransformAccessorGroup
        {
            TransformAccessorGroup(
                sgTransform* owner,
                TransformReader localRead,
                TransformWriter localWrite,
                TransformReader worldRead,
                TransformWriter worldWrite);
            void BindOwner(sgTransform* newOwner);

            friend class sgTransform;

          public:
            TransformVectorAccessor local;
            TransformVectorAccessor world;

            TransformAccessorGroup() = default;
            TransformAccessorGroup(const TransformAccessorGroup&) = delete;
            TransformAccessorGroup& operator=(const TransformAccessorGroup&) = delete;
        };

        TransformAccessorGroup position;
        TransformAccessorGroup rotation;
        TransformAccessorGroup scale;

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

        template <class Inspector>
        void define_editor_fields(Inspector& i)
        {
            i.field("Position", m_positionLocal);
            i.field("Rotation", m_rotationLocal);
            i.field("Scale", m_scaleLocal);
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
