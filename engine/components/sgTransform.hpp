//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "cereal/cereal.hpp"
#include "entt/entt.hpp"
#include "raylib.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace sage
{
    class TransformSystem;

    class sgTransform
    {
        entt::entity m_entity = entt::null;
        TransformSystem* m_transformSystem = nullptr;
        entt::entity m_parent = entt::null;
        std::vector<entt::entity> m_children{};

        // Transient: the saving registry's entity-index for our parent, populated by cereal load().
        // ResolveSerializedParent() consumes it and clears back to the null sentinel.
        std::uint32_t m_savedParentId = serializedNullId();

        static constexpr std::uint32_t serializedNullId()
        {
            return static_cast<std::uint32_t>(
                entt::entt_traits<entt::entity>::to_entity(entt::entity{entt::null}));
        }

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
        // happens automatically; reads return the cached value directly.
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
                float Vector3::*axis = nullptr;

                operator float() const
                {
                    return parent->value.*axis;
                }
                Axis& operator=(float v)
                {
                    Vector3 next = parent->value;
                    next.*axis = v;
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
            VectorField& operator=(const Vector3& v)
            {
                (owner_->*Write)(v);
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

        LocalWorldPair<&sgTransform::writeLocalPos, &sgTransform::writeWorldPos> position;
        LocalWorldPair<&sgTransform::writeLocalRot, &sgTransform::writeWorldRot> rotation;
        LocalWorldPair<&sgTransform::writeLocalScale, &sgTransform::writeWorldScale> scale;

        Vector3 direction{};

        Ray movementDirectionDebugLine{};

        // Forwards to TransformSystem::SetParent so child lists and the
        // local/world sync run automatically.
        void SetParent(entt::entity newParent);

        // Bin loaders call this after all entities are restored. The map keys are
        // entity indices from the source registry (saved via sage::serializer::entity)
        // and the values are the corresponding live entities in this registry.
        void ResolveSerializedParent(const std::unordered_map<std::uint32_t, entt::entity>& idMap);

        template <class Archive>
        void save(Archive& archive) const
        {
            using traits = entt::entt_traits<entt::entity>;
            const std::uint32_t parentId = static_cast<std::uint32_t>(traits::to_entity(m_parent));
            archive(
                position.world.value,
                rotation.world.value,
                scale.world.value,
                position.local.value,
                rotation.local.value,
                scale.local.value,
                parentId);
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
                scale.local.value,
                m_savedParentId);
        }

        // Inspector authors get the proxy fields straight — assignment in the inspector
        // routes through TransformSystem via the VectorField overload of `field()`.
        template <class Inspector>
        void define_editor_fields(Inspector& i)
        {
            i.field("Position", position.local);
            i.field("Rotation", rotation.local);
            i.field("Scale", scale.local);
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
