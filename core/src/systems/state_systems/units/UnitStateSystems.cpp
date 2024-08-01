//
// Created by Steve Wheeler on 08/06/2024.
//

#include "UnitStateSystems.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

namespace sage
{
	void UnitStateSystems::Update()
	{
		for (auto& system : systems)
		{
			system->Update();
		}
	}

	void UnitStateSystems::Draw3D()
	{
		for (auto& system : systems)
		{
			system->Draw3D();
		}
	}

	UnitStateSystems::UnitStateSystems(entt::registry* _registry,
			Cursor* _cursor,
			ControllableActorSystem* _controllableActorSystem,
			ActorMovementSystem* _actorMovementSystem,
			CollisionSystem* _collisionSystem,
			NavigationGridSystem* _navigationGridSystem) :
			registry(_registry),
			cursor(_cursor),
			controllableActorSystem(_controllableActorSystem),
			waveMobDefaultSubSystem(std::make_unique<WaveMobDefaultStateSystem>(_registry, _actorMovementSystem)),
			playerDefaultSubSystem(std::make_unique<PlayerDefaultStateSystem>(_registry, _actorMovementSystem)),
			playerCombatLogicSubSystem(std::make_unique<PlayerCombatStateSystem>(_registry, _controllableActorSystem)),
			waveMobCombatLogicSubSystem(std::make_unique<WaveMobCombatStateSystem>(_registry, _actorMovementSystem, _collisionSystem, _navigationGridSystem))
	{
		systems.push_back(waveMobDefaultSubSystem.get());
		systems.push_back(playerDefaultSubSystem.get());
		systems.push_back(playerCombatLogicSubSystem.get());
		systems.push_back(waveMobCombatLogicSubSystem.get());
	}
} // sage