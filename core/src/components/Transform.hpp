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
		Vector3 m_direction{};
		Vector3 m_rotation{};
		float m_scale = 1.0f;
    public:
		float movementSpeed = 0.35f;

		Ray movementDirectionDebugLine{};

		Transform() = default;
		Transform(const Transform&) = delete;
		Transform& operator=(const Transform&) = delete;

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(
				CEREAL_NVP(position.x),
				CEREAL_NVP(position.y),
				CEREAL_NVP(position.z),
				CEREAL_NVP(rotation.x),
				CEREAL_NVP(rotation.y),
				CEREAL_NVP(rotation.z),
				CEREAL_NVP(scale));
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(position.x,
			        position.y,
			        position.z,
			        rotation.x,
			        rotation.y,
			        rotation.z,
			        scale);
		}

		entt::sigh<void(entt::entity)> onPositionUpdate{};
		entt::sigh<void(entt::entity)> onStartMovement{};
		entt::sigh<void(entt::entity)> onFinishMovement{};
        entt::sigh<void(entt::entity)> onDestinationReached{};
		entt::sigh<void(entt::entity)> onMovementCancel{};

		[[nodiscard]] Matrix GetMatrixNoRot() const;
		[[nodiscard]] Matrix GetMatrix() const;
		[[nodiscard]] Vector3 forward() const;
	};
}
