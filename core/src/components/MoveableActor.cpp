//
// Created by Steve Wheeler on 16/06/2024.
//

#include "MoveableActor.hpp"

#include "EntityReflectionSignalRouter.hpp"
#include "sgTransform.hpp"
#include "slib.hpp"

namespace sage
{
    void FollowTarget::targetReachedDestination() const
    {
        // TODO: Replace with a hook?
        onTargetDestinationReached.publish(self, targetActor);
    }

    void FollowTarget::targetPathChanged() const
    {
        // TODO: Replace with a hook?
        onTargetPathChanged.publish(self, targetActor);
    }

    void FollowTarget::targetMovementCancelled() const
    {
        // TODO: Replace with a hook?
        onTargetMovementCancelled.publish(self, targetActor);
    }

    FollowTarget::~FollowTarget()
    {
        onTargetPathChangedCnx.release();
        onTargetDestinationReachedCnx.release();
        onTargetMovementCancelledCnx.release();
    }

    FollowTarget::FollowTarget(
        entt::registry* _registry, const entt::entity _self, const entt::entity _targetActor)
        : registry(_registry), self(_self), timeStarted(GetTime()), targetActor(_targetActor)
    {
        auto& moveable = _registry->get<MoveableActor>(_targetActor);
        entt::sink sink{moveable.onPathChanged};
        onTargetPathChangedCnx = sink.connect<&FollowTarget::targetPathChanged>(this);
        entt::sink sink2{moveable.onMovementCancel};
        onTargetMovementCancelledCnx = sink2.connect<&FollowTarget::targetMovementCancelled>(this);
        entt::sink sink3{moveable.onDestinationReached};
        onTargetDestinationReachedCnx = sink3.connect<&FollowTarget::targetReachedDestination>(this);
    }
} // namespace sage
