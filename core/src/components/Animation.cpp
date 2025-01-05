//
// Created by Steve Wheeler on 06/04/2024.
//

#include "Animation.hpp"

namespace sage
{

    void Animation::ChangeAnimationByParams(AnimationParams params)
    {
        if (params.oneShot)
        {
            PlayOneShot(params.animEnum, params.animSpeed);
            return;
        }
        ChangeAnimationByEnum(params.animEnum, params.animSpeed);
    }

    void Animation::ChangeAnimationByEnum(AnimationEnum animEnum, int _animSpeed)
    {
        assert(animationMap.contains(animEnum));
        ChangeAnimation(animationMap.at(animEnum), _animSpeed);
    }

    void Animation::ChangeAnimationByEnum(AnimationEnum animEnum)
    {
        ChangeAnimationByEnum(animEnum, 1);
    }

    void Animation::ChangeAnimation(int index)
    {
        ChangeAnimation(index, 1);
    }

    void Animation::ChangeAnimation(int index, int _animSpeed)
    {
        auto& a = oneShotMode ? prev : current;

        if (a.index == index) return;
        a.speed = _animSpeed;
        a.index = index;
        a.currentFrame = 0;
    }

    void Animation::PlayOneShot(AnimationEnum animEnum, int _animSpeed)
    {
        PlayOneShot(animationMap.at(animEnum), _animSpeed);
    }

    void Animation::PlayOneShot(int index, int _animSpeed)
    {
        if (!oneShotMode)
        {
            oneShotMode = true;
            prev = current;
        }

        current.index = index;
        current.speed = _animSpeed;
        current.currentFrame = 0;
        current.lastFrame = 0;
    }

    void Animation::RestoreAfterOneShot()
    {
        oneShotMode = false;
        current = prev;
        prev = {};
    }

    Animation::Animation(const AssetID id)
        : onAnimationEnd(std::make_unique<Event<entt::entity>>()),
          onAnimationStart(std::make_unique<Event<entt::entity>>()),
          onAnimationUpdated(std::make_unique<Event<entt::entity>>())
    {
        animsCount = 0;
        animations = ResourceManager::GetInstance().GetModelAnimation(id, &animsCount);
    }
} // namespace sage
