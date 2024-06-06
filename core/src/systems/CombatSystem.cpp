//
// Created by Steve Wheeler on 03/06/2024.
//

#include "CombatSystem.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"

#include <iostream>

#include "raymath.h"

namespace sage
{


void CombatSystem::Update()
{
    auto view = registry->view<CombatableActor>();
    for (auto& entity : view) 
    {
        auto& c = registry->get<CombatableActor>(entity);
		if (c.actorType == CombatableActorType::PLAYER)
		{
			playerCombatLogicSubSystem->Update(entity);
		}
		else if (c.actorType == CombatableActorType::WAVEMOB)
		{
			waveMobCombatLogicSubSystem->Update(entity);
		}
    }
}

CombatSystem::CombatSystem(entt::registry *_registry,
                           Cursor *_cursor,
                           ControllableActorMovementSystem* _actorMovementSystem,
                           TransformSystem* _transformSystem,
                           CollisionSystem* _collisionSystem) :
                           BaseSystem<CombatableActor>(_registry),
                               cursor(_cursor),
                               actorMovementSystem(_actorMovementSystem),
                               playerCombatLogicSubSystem(std::make_unique<PlayerCombatLogicSubSystem>(_registry, _actorMovementSystem, _cursor)),
                               waveMobCombatLogicSubSystem(std::make_unique<WaveMobCombatLogicSubSystem>(_registry, _transformSystem, _collisionSystem))
{
}
} // sage