//
// Created by Steve Wheeler on 21/02/2024.
//

#pragma once

#include "raylib.h"
#include "entt/entt.hpp"
#include "cereal/cereal.hpp"

namespace sage
{
	class Transform
	{
		Vector3 m_position{};
		Vector3 m_rotation{};
		float m_scale = 1.0f;
    public:
		Vector3 direction{};
		float movementSpeed = 0.35f;

		Ray movementDirectionDebugLine{};

		Transform() = default;
		Transform(const Transform&) = delete;
		Transform& operator=(const Transform&) = delete;

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(
				CEREAL_NVP(m_position.x),
				CEREAL_NVP(m_position.y),
				CEREAL_NVP(m_position.z),
				CEREAL_NVP(m_rotation.x),
				CEREAL_NVP(m_rotation.y),
				CEREAL_NVP(m_rotation.z),
				CEREAL_NVP(m_scale));
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(m_position.x,
			        m_position.y,
			        m_position.z,
			        m_rotation.x,
			        m_rotation.y,
			        m_rotation.z,
			        m_scale);
		}

		entt::sigh<void(entt::entity)> onPositionUpdate{};
		entt::sigh<void(entt::entity)> onStartMovement{};
		entt::sigh<void(entt::entity)> onFinishMovement{};
        entt::sigh<void(entt::entity)> onDestinationReached{};
		entt::sigh<void(entt::entity)> onMovementCancel{};

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
}
