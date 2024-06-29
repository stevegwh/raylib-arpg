//
// Created by Steve Wheeler on 03/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "Cursor.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "components/CombatableActor.hpp"
#include "PlayerCombatLogicSubSystem.hpp"
#include "WaveMobCombatLogicSubSystem.hpp"
#include "systems/StateMachineSystem.hpp"

namespace sage
{
class ActorMovementSystem; // forward dec
class CollisionSystem; // forward dec
class CombatStateSystem : public BaseSystem<CombatableActor>
{
    StateMachineSystem* stateMachineSystem;
    Cursor* cursor;
    ControllableActorSystem* actorMovementSystem;
public:
	std::unique_ptr<PlayerCombatLogicSubSystem> playerCombatLogicSubSystem;
	std::unique_ptr<WaveMobCombatLogicSubSystem> waveMobCombatLogicSubSystem;
    CombatStateSystem(entt::registry *_registry,
			Cursor *_cursor,
			StateMachineSystem* _stateMachineSystem,
			ControllableActorSystem* _actorMovementSystem,
			ActorMovementSystem* _transformSystem,
			CollisionSystem* _collisionSystem,
            NavigationGridSystem* _navigationGridSystem);
    void Update();
    void Draw3D();
};

} // sage
