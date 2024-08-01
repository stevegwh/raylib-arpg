#include "PlayerDefaultStateSystem.hpp"

#include "components/Animation.hpp"

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
	}
}
