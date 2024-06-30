//
// Created by Steve Wheeler on 16/06/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

#include <deque>
#include <optional>

namespace sage
{
	struct MoveableActor
	{
		entt::entity lastHitActor = entt::null;
		Vector3 hitActorLastPos{};
		std::deque<Vector3> path{};
		std::optional<Vector3> destination{};
	};
} // sage
