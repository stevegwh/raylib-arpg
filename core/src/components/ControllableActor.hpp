//
// Created by Steve Wheeler on 29/02/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
	struct ControllableActor
	{
		int pathfindingBounds = 25; // The max range the actor can pathfind at one time.
		float checkTargetPosTimer;
		float checkTargetPosThreshold = 1.0f;
		entt::entity targetActor = entt::null; // An actor that is the target for pathfinding etc.
		Vector3 targetActorPos{};
	};
}
