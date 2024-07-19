//
// Created by Steve Wheeler on 08/06/2024.
//

#include "components/Animation.hpp"
#include "components/states/StateEnemyDefault.hpp"
#include "components/sgTransform.hpp"
#include "WaveMobDefaultSubSystem.hpp"

namespace sage
{
	void WaveMobDefaultSubSystem::OnStateEnter(entt::entity entity) const
	{
		Vector3 target = { 52, 0, -10 };
		auto& a = registry->get<MoveableActor>(entity);
		auto& t = registry->get<sgTransform>(entity);
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
		actorMovementSystem->PathfindToLocation(entity, target);
	}

	void WaveMobDefaultSubSystem::OnStateExit(entt::entity entity) const
	{
		//actorMovementSystem->CancelMovement(entity);
	}

	void WaveMobDefaultSubSystem::Update()
	{
	}

	WaveMobDefaultSubSystem::WaveMobDefaultSubSystem(entt::registry* _registry,
		ActorMovementSystem* _actorMovementSystem) :
		StateMachineSystem(_registry),
		actorMovementSystem(_actorMovementSystem)
	{
		registry->on_construct<StateEnemyDefault>().connect<&WaveMobDefaultSubSystem::OnStateEnter>(this);
		registry->on_destroy<StateEnemyDefault>().connect<&WaveMobDefaultSubSystem::OnStateExit>(this);
	}
} // sage
