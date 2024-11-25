//
// Created by Steve Wheeler on 16/06/2024.
//

#include "MoveableActor.hpp"

#include "EntityReflectionSignalRouter.hpp"
#include "sgTransform.hpp"
#include "slib.hpp"

namespace sage
{

    void FollowTarget::checkTargetPos()
    {
        // Reacts to the target transform's position update, but does fire each time for performance reasons.
        auto current = GetTime();
        if (current > timeStarted + timerThreshold)
        {
            const auto& targetTrans = registry->get<sgTransform>(targetActor);
            if (const auto targetCurrentPos = targetTrans.GetWorldPos();
                !AlmostEquals(targetCurrentPos, targetPrevPos))
            {
                targetPrevPos = targetCurrentPos;
                onPositionUpdate.publish(self, targetActor);
            }
            timeStarted = current;
        }
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
        onTargetPosUpdateCnx = sink.connect<&FollowTarget::checkTargetPos>(this);
    }

    FollowTarget::FollowTarget(const FollowTargetParams& params)
        : registry(params.registry), self(params.self), targetActor(params.targetActor)
    {
        auto& targetTrans = params.registry->get<sgTransform>(params.targetActor);
        entt::sink sink{targetTrans.onPositionUpdate};
        onTargetPosUpdateCnx = sink.connect<&FollowTarget::checkTargetPos>(this);
    }
} // namespace sage
