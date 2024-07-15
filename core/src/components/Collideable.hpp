//
// Created by Steve Wheeler on 18/02/2024.
//

#pragma once

#include "raylib.h"
#include "entt/entt.hpp"
#include "cereal/cereal.hpp"
#include "raylib-cereal.hpp"

namespace sage
{
	enum class CollisionLayer
	{
		DEFAULT,
		FLOOR,
		BUILDING,
		NAVIGATION,
		PLAYER,
		NPC,
		ENEMY,
		BOYD,
        TERRAIN,
		COUNT // Must always be last
	};

	struct CollisionInfo
	{
		entt::entity collidedEntityId{};
		BoundingBox collidedBB{};
		RayCollision rlCollision{};
		CollisionLayer collisionLayer{};
	};

	struct Collideable
	{
		BoundingBox localBoundingBox{}; // BoundingBox in local space
		BoundingBox worldBoundingBox{}; // BoundingBox in world space (bb * world mat)
		CollisionLayer collisionLayer = CollisionLayer::DEFAULT;
		bool debugDraw = false;

		explicit Collideable(BoundingBox _boundingBox);
		Collideable() = default;
		Collideable(const Collideable&) = delete;
		Collideable& operator=(const Collideable&) = delete;

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(
				localBoundingBox,
				worldBoundingBox,
				collisionLayer);
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(
				localBoundingBox,
				worldBoundingBox,
				collisionLayer);
		}
	};
}
