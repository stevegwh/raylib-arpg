//
// Created by Steve Wheeler on 16/06/2024.
//

#pragma once

#include "raylib.h"
#include "NavigationGridSquare.hpp"
#include <entt/entt.hpp>

#include <deque>
#include <optional>
#include <vector>

namespace sage
{
	struct MoveableActor
	{
		entt::entity lastHitActor = entt::null;
		Vector3 hitActorLastPos{};
		std::deque<Vector3> path{};
		bool isMoving() const
		{
			return !path.empty();
		}
        std::vector<GridSquare> debugRay;
	};
} // sage
