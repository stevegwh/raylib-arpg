#include "PlayerDefaultStateSystem.hpp"

#include "components/Animation.hpp"
#include "components/states/StatePlayerDefault.hpp"

namespace sage
{
	void PlayerDefaultStateSystem::Update()
	{
	}

	void PlayerDefaultStateSystem::Draw3D()
	{
	}

	void PlayerDefaultStateSystem::OnStateEnter(entt::entity entity)
	{
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
	}

	void PlayerDefaultStateSystem::OnStateExit(entt::entity entity)
	{
		actorMovementSystem->CancelMovement(entity);
	}

	PlayerDefaultStateSystem::PlayerDefaultStateSystem(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem) : 
		StateMachineSystem(_registry),
		actorMovementSystem(_actorMovementSystem)
	{
		registry->on_construct<StatePlayerDefault>().connect<&PlayerDefaultStateSystem::OnStateEnter>(this);
		registry->on_destroy<StatePlayerDefault>().connect<&PlayerDefaultStateSystem::OnStateExit>(this);
	}
}
