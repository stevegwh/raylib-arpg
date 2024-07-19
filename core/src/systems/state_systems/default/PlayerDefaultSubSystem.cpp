#include "PlayerDefaultSubSystem.hpp"
#include "PlayerDefaultSubSystem.hpp"

#include "components/Animation.hpp"
#include "components/states/StatePlayerDefault.hpp"

namespace sage
{
	void PlayerDefaultSubSystem::Update()
	{
	}

	void PlayerDefaultSubSystem::Draw3D(entt::entity entity)
	{
	}

	void PlayerDefaultSubSystem::OnStateEnter(entt::entity entity)
	{
		auto& animation = registry->get<Animation>(entity);
		animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
	}

	void PlayerDefaultSubSystem::OnStateExit(entt::entity entity)
	{
		actorMovementSystem->CancelMovement(entity);
	}

	PlayerDefaultSubSystem::PlayerDefaultSubSystem(entt::registry* _registry, ActorMovementSystem* _actorMovementSystem) : 
		StateMachineSystem(_registry),
		actorMovementSystem(_actorMovementSystem)
	{
		registry->on_construct<StatePlayerDefault>().connect<&PlayerDefaultSubSystem::OnStateEnter>(this);
		registry->on_destroy<StatePlayerDefault>().connect<&PlayerDefaultSubSystem::OnStateExit>(this);
	}
}
