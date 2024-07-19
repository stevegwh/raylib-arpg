//
// Created by Steve Wheeler on 03/06/2024.
//

#include "CombatStateSystems.hpp"

#include "components/HealthBar.hpp"
#include "components/CombatableActor.hpp"
#include "components/states/StatePlayerCombat.hpp"

#include <iostream>

namespace sage
{
	void CombatStateSystems::Draw3D()
	{
		for (auto& system : systems)
		{
			system->Draw3D();
		}
	}


	void CombatStateSystems::Update()
	{
		for (auto& system : systems)
		{
			system->Update();
		}
	}

	CombatStateSystems::CombatStateSystems(entt::registry* _registry,
		Cursor* _cursor,
		ControllableActorSystem* _actorMovementSystem,
		ActorMovementSystem* _transformSystem,
		CollisionSystem* _collisionSystem,
		NavigationGridSystem* _navigationGridSystem) :
		registry(_registry),
		cursor(_cursor),
		actorMovementSystem(_actorMovementSystem),
		playerCombatLogicSubSystem(
			std::make_unique<PlayerCombatStateSystem>(_registry, _actorMovementSystem)),
		waveMobCombatLogicSubSystem(std::make_unique<WaveMobCombatStateSystem>(
			_registry, _transformSystem, _collisionSystem, _navigationGridSystem))
	{
		systems.push_back(playerCombatLogicSubSystem.get());
		systems.push_back(waveMobCombatLogicSubSystem.get());
	}
} // sage
