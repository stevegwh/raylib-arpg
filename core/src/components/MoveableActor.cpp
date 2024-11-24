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
        if (const double timeLapsed = GetTime(); timeLapsed - lastCheckTime > timerThreshold)
        {
            const auto& targetTrans = registry->get<sgTransform>(targetActor);
            if (const auto targetCurrentPos = targetTrans.GetWorldPos();
                !AlmostEquals(targetCurrentPos, targetPrevPos))
            {
                targetPrevPos = targetCurrentPos;
                onTargetPosUpdate.publish(self, targetActor);
            }
            lastCheckTime = timeLapsed;
        }
    }

    FollowTarget::~FollowTarget()
    {
        reflectionSignalRouter->RemoveHook(onTargetPosUpdateHookId);
    }

    FollowTarget::FollowTarget(
        entt::registry* _registry,
        EntityReflectionSignalRouter* _reflectionSignalRouter,
        const entt::entity _self,
        const entt::entity _targetActor)
        : registry(_registry),
          self(_self),
          reflectionSignalRouter(_reflectionSignalRouter),
          targetActor(_targetActor)
    {
        auto& targetTrans = _registry->get<sgTransform>(_targetActor);
        onTargetPosUpdateHookId = reflectionSignalRouter->CreateHook<entt::entity>(
            _self, targetTrans.onPositionUpdate, onTargetPosUpdate);
    };
} // namespace sage
