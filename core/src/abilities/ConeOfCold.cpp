//
// Created by Steve Wheeler on 21/07/2024.
//

#include "ConeOfCold.hpp"
#include "raylib.h"
#include "components/sgTransform.hpp"
#include "components/CombatableActor.hpp"
#include "components/Animation.hpp"

namespace sage
{
	void ConeOfCold::Execute(entt::entity actor)
	{

	}

	void ConeOfCold::Update(entt::entity actor)
	{
		
	}

	ConeOfCold::ConeOfCold(entt::registry* _registry, CollisionSystem* _collisionSystem, TimerManager* _timerManager) :
			Ability(_registry, _collisionSystem, _timerManager)
	{
	}
}
