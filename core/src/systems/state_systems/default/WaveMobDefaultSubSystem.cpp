//
// Created by Steve Wheeler on 08/06/2024.
//

#include "WaveMobDefaultSubSystem.hpp"
#include "components/states/StateEnemyDefault.hpp"
#include "components/Transform.hpp"
#include "components/Animation.hpp"

#include <iostream>

namespace sage
{
void WaveMobDefaultSubSystem::OnComponentEnabled(entt::entity entity) const
{
	Vector3 target = { -5.0, 0, 0 };
	auto& a = registry->get<MoveableActor>(entity);
	auto& t = registry->get<Transform>(entity);
	auto& animation = registry->get<Animation>(entity);
	animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
	actorMovementSystem->PathfindToLocation(entity, target);
}

void WaveMobDefaultSubSystem::OnComponentDisabled(entt::entity entity) const
{
	//actorMovementSystem->CancelMovement(entity);
}

void WaveMobDefaultSubSystem::Update()
{
}

WaveMobDefaultSubSystem::WaveMobDefaultSubSystem(entt::registry* _registry,
                                                 StateMachineSystem* _stateMachineSystem,
                                                 ActorMovementSystem* _actorMovementSystem) :
    registry(_registry), 
    stateMachineSystem(_stateMachineSystem),
    actorMovementSystem(_actorMovementSystem)
{
	registry->on_construct<StateEnemyDefault>().connect<&WaveMobDefaultSubSystem::OnComponentEnabled>(this);
	registry->on_destroy<StateEnemyDefault>().connect<&WaveMobDefaultSubSystem::OnComponentDisabled>(this);
}
} // sage