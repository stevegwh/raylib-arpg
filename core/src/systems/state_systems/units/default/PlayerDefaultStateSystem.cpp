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

    void PlayerDefaultStateSystem::OnComponentAdded(entt::entity entity)
    {
        auto& animation = registry->get<Animation>(entity);
        animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
    }

    void PlayerDefaultStateSystem::OnComponentRemoved(entt::entity entity)
    {
        actorMovementSystem->CancelMovement(entity);
    }

    PlayerDefaultStateSystem::PlayerDefaultStateSystem(entt::registry* _registry,
                                                       ActorMovementSystem* _actorMovementSystem)
        : StateMachine(_registry), actorMovementSystem(_actorMovementSystem)
    {
    }
} // namespace sage
