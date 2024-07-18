#include "PlayerDefaultSubSystem.hpp"

#include "components/Animation.hpp"
#include "components/states/StatePlayerDefault.hpp"

namespace sage
{
	void PlayerDefaultSubSystem::Update()
	{
	}

	void PlayerDefaultSubSystem::OnStateAdded(entt::entity entity) const
	{
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
	}

	void PlayerDefaultSubSystem::OnStateRemoved(entt::entity entity) const
	{
		actorMovementSystem->CancelMovement(entity);
	}

	PlayerDefaultSubSystem::PlayerDefaultSubSystem(entt::registry* _registry, StateMachineSystem* _stateMachineSystem,
	                                               ActorMovementSystem* _actorMovementSystem) :
		registry(_registry),
		stateMachineSystem(_stateMachineSystem),
		actorMovementSystem(_actorMovementSystem)
	{
		registry->on_construct<StatePlayerDefault>().connect<&PlayerDefaultSubSystem::OnStateAdded>(this);
		registry->on_destroy<StatePlayerDefault>().connect<&PlayerDefaultSubSystem::OnStateRemoved>(this);
	}
}
