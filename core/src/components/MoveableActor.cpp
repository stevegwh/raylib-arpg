//
// Created by Steve Wheeler on 16/06/2024.
//

#include "MoveableActor.hpp"

#include "EntityReflectionSignalRouter.hpp"
#include "sgTransform.hpp"

namespace sage
{

    bool FollowTarget::ShouldCheckTargetPos()
    {
        double timeLapsed = GetTime();
        if (timeLapsed - lastCheckTime > timerThreshold)
        {
            lastCheckTime = timeLapsed;
            return true;
        }
        return false;
    }

    FollowTarget::~FollowTarget()
    {
        reflectionSignalRouter->RemoveHook(onTargetPosUpdateHookId);
    }

    FollowTarget::FollowTarget(
        entt::registry* _registry,
        EntityReflectionSignalRouter* _reflectionSignalRouter,
        const entt::entity self,
        const entt::entity _targetActor)
        : reflectionSignalRouter(_reflectionSignalRouter), targetActor(_targetActor)
    {
        auto& targetTrans = _registry->get<sgTransform>(_targetActor);
        onTargetPosUpdateHookId = reflectionSignalRouter->CreateHook<entt::entity>(
            self, targetTrans.onPositionUpdate, onTargetPosUpdate);
    };
} // namespace sage
