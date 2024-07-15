//
// Created by Steve Wheeler on 08/06/2024.
//

#include "components/Animation.hpp"
#include "components/states/StateEnemyDefault.hpp"
#include "components/sgTransform.hpp"
#include "WaveMobDefaultSubSystem.hpp"

namespace sage
{
	void WaveMobDefaultSubSystem::OnComponentEnabled(entt::entity entity) const
	{
		Vector3 target = {52, 0, -10};
		auto& a = registry->get<MoveableActor>(entity);
		auto& t = registry->get<sgTransform>(entity);
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
