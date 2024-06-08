//
// Created by Steve Wheeler on 03/06/2024.
//

#pragma once

#include "entt/entt.hpp"

#include "Cursor.hpp"
#include "systems/ControllableActorMovementSystem.hpp"
#include "components/CombatableActor.hpp"
#include "PlayerCombatLogicSubSystem.hpp"
#include "WaveMobCombatLogicSubSystem.hpp"
#include "systems/StateMachineSystem.hpp"

namespace sage
{
class TransformSystem; // forward dec
class CollisionSystem; // forward dec
class CombatStateSystem : public BaseSystem<CombatableActor>
{
    StateMachineSystem* stateMachineSystem;
    Cursor* cursor;
    ControllableActorMovementSystem* actorMovementSystem;
public:
	std::unique_ptr<PlayerCombatLogicSubSystem> playerCombatLogicSubSystem;
	std::unique_ptr<WaveMobCombatLogicSubSystem> waveMobCombatLogicSubSystem;
    CombatStateSystem(entt::registry *_registry,
                      Cursor *_cursor,
                      StateMachineSystem* _stateMachineSystem,
                      ControllableActorMovementSystem* _actorMovementSystem,
                      TransformSystem* _transformSystem,
                      CollisionSystem* _collisionSystem);
    void Update();
    void Draw3D();
};

} // sage
