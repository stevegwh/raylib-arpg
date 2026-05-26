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
        entt::entity m_entity = entt::null;
        TransformSystem* m_transformSystem = nullptr;
        std::vector<entt::entity> m_children{};

        // One-line forwarders to TransformSystem. Defined in the .cpp so
        // TransformSystem can stay forward-declared in this header.
        void writeLocalPos(const Vector3& v);
        void writeWorldPos(const Vector3& v);
        void writeLocalRot(const Vector3& v);
        void writeWorldRot(const Vector3& v);
        void writeLocalScale(const Vector3& v);
        void writeWorldScale(const Vector3& v);

        using Writer = void (sgTransform::*)(const Vector3&);

        void Bind(TransformSystem* transformSystem, entt::entity entity);
        void rebindProxies();

      public:
        // Proxy field with its cached Vector3 living inside.
        // Assignment routes through TransformSystem (via `Write`) so dirty propagation
        // happens automatically; reads return the cached value_ directly.
        template <Writer Write>
        class VectorField
        {
            Vector3 value{};
            sgTransform* owner_ = nullptr;
            friend class sgTransform;
            friend class TransformSystem;

          public:
            struct Axis
            {
                VectorField* parent = nullptr;
                float Vector3::* axis = nullptr;

                operator float() const
                {
                    return parent->value.*axis;
                }
                Axis& operator=(float value)
                {
                    Vector3 next = parent->value;
                    next.*axis = value;
                    (parent->owner_->*Write)(next);
                    return *this;
                }
                Axis& operator=(const Axis& rhs)
                {
                    return *this = static_cast<float>(rhs);
                }
            };

            Axis x{this, &Vector3::x};
            Axis y{this, &Vector3::y};
            Axis z{this, &Vector3::z};

            VectorField() = default;
            VectorField(const VectorField&) = delete;
            VectorField(VectorField&&) = delete;
            VectorField& operator=(VectorField&&) = delete;

            operator const Vector3&() const
            {
                return value;
            }
            [[nodiscard]] const Vector3& Get() const
            {
                return value;
            }
            VectorField& operator=(const Vector3& value)
            {
                (owner_->*Write)(value);
                return *this;
            }
            VectorField& operator=(const VectorField& rhs)
            {
                if (this == &rhs) return *this;
                return *this = rhs.Get();
            }
        };

        template <Writer LocalWrite, Writer WorldWrite>
        struct LocalWorldPair
        {
            VectorField<LocalWrite> local{};
            VectorField<WorldWrite> world{};
        };

        // Parent is an entity reference; assigning routes through TransformSystem::SetParent
        // so child lists and the local/world sync happen automatically.
        class ParentField
        {
            entt::entity value_ = entt::null;
            sgTransform* owner_ = nullptr;
            friend class sgTransform;
            friend class TransformSystem;

          public:
            ParentField() = default;
            ParentField(const ParentField&) = delete;
            ParentField(ParentField&&) = delete;
            ParentField& operator=(ParentField&&) = delete;

            operator entt::entity() const
            {
                return value_;
            }
            [[nodiscard]] entt::entity Get() const
            {
                return value_;
            }
            ParentField& operator=(entt::entity newParent);
            ParentField& operator=(entt::null_t)
            {
                return *this = entt::entity{entt::null};
            }
            ParentField& operator=(const ParentField& rhs)
            {
                if (this == &rhs) return *this;
                return *this = rhs.value_;
            }
        };

        LocalWorldPair<&sgTransform::writeLocalPos, &sgTransform::writeWorldPos> position;
        LocalWorldPair<&sgTransform::writeLocalRot, &sgTransform::writeWorldRot> rotation;
        LocalWorldPair<&sgTransform::writeLocalScale, &sgTransform::writeWorldScale> scale;
        ParentField parent;

        Vector3 direction{};

        Ray movementDirectionDebugLine{};

        template <class Archive>
        void save(Archive& archive) const
        {
            archive(
                position.world.value,
                rotation.world.value,
                scale.world.value,
                position.local.value,
                rotation.local.value,
                scale.local.value);
        }

        template <class Archive>
        void load(Archive& archive)
        {
            archive(
                position.world.value,
                rotation.world.value,
                scale.world.value,
                position.local.value,
                rotation.local.value,
                scale.local.value);
        }

        template <class Inspector>
        void define_editor_fields(Inspector& i)
        {
            i.field("Position", position.local.value);
            i.field("Rotation", rotation.local.value);
            i.field("Scale", scale.local.value);
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

        sgTransform();
        sgTransform(const sgTransform& rhs);
        sgTransform& operator=(const sgTransform& rhs);
        sgTransform(sgTransform&& rhs) noexcept;
        sgTransform& operator=(sgTransform&& rhs) noexcept;

        friend class TransformSystem;
    };
} // namespace sage
