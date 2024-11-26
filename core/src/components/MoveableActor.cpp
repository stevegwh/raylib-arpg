//
// Created by Steve Wheeler on 16/06/2024.
//

#include "MoveableActor.hpp"

#include "EntityReflectionSignalRouter.hpp"
#include "sgTransform.hpp"
#include "slib.hpp"

namespace sage
{

    void FollowTarget::changePath() const
    {
        // TODO: Should replace with a hook
        onPathChanged.publish(self, targetActor);
    }

    FollowTarget::~FollowTarget()
    {
        onTargetPosUpdateCnx.release();
    }

    FollowTarget::FollowTarget(
        entt::registry* _registry, const entt::entity _self, const entt::entity _targetActor)
        : registry(_registry), self(_self), timeStarted(GetTime()), targetActor(_targetActor)
    {
        auto& targetTrans = _registry->get<sgTransform>(_targetActor);
        entt::sink sink{targetTrans.onPositionUpdate};
        onTargetPosUpdateCnx = sink.connect<&FollowTarget::changePath>(this);
    }
} // namespace sage
