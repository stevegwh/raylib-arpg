//
// Created by Steve Wheeler on 16/06/2024.
//

#include "MoveableActor.hpp"

#include "sgTransform.hpp"
#include "slib.hpp"

namespace sage
{
    FollowTarget::~FollowTarget()
    {
        onTargetPathChangedCnx.UnSubscribe();
        onTargetDestinationReachedCnx.UnSubscribe();
        onTargetMovementCancelledCnx.UnSubscribe();
    }

    FollowTarget::FollowTarget(
        entt::registry* _registry, const entt::entity _self, const entt::entity _targetActor)
        : registry(_registry), self(_self), timeStarted(GetTime()), targetActor(_targetActor)
    {
        auto& moveable = _registry->get<MoveableActor>(_targetActor);

        onTargetPathChangedCnx = moveable.onPathChanged->Subscribe(
            [this](entt::entity) { onTargetPathChanged->Publish(self, targetActor); });
        onTargetMovementCancelledCnx = moveable.onMovementCancel->Subscribe(
            [this](entt::entity) { onTargetMovementCancelled->Publish(self, targetActor); });
        onTargetDestinationReachedCnx = moveable.onDestinationReached->Subscribe(
            [this](entt::entity) { onTargetDestinationReached->Publish(self, targetActor); });
    }

    MoveableActor::MoveableActor()
        : onStartMovement(std::make_unique<Event<entt::entity>>()),
          onDestinationReached(std::make_unique<Event<entt::entity>>()),
          onDestinationUnreachable(std::make_unique<Event<entt::entity, Vector3>>()),
          onPathChanged(std::make_unique<Event<entt::entity>>()),
          onMovementCancel(std::make_unique<Event<entt::entity>>())
    {
    }

} // namespace sage
