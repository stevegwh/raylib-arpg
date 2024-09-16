//
// Created by Steve Wheeler on 06/04/2024.
//

#include "Animation.hpp"

namespace sage
{
    void Animation::ChangeAnimationByParams(AnimationParams params)
    {
        ChangeAnimationByEnum(params.animEnum, params.animSpeed, params.oneShot);
    }

    void Animation::ChangeAnimationByEnum(AnimationEnum animEnum, int _animSpeed, bool _oneShot = false)
    {
        assert(animationMap.contains(animEnum));
        ChangeAnimation(animationMap.at(animEnum), _animSpeed, _oneShot);
    }

    void Animation::ChangeAnimationByEnum(AnimationEnum animEnum, bool _oneShot = false)
    {
        ChangeAnimationByEnum(animEnum, 1, _oneShot);
    }

    void Animation::ChangeAnimation(int index, int _animSpeed, bool _oneShot = false)
    {
        if (animIndex == index && !_oneShot) return;
        animSpeed = _animSpeed;
        animIndex = index;
        oneShot = _oneShot;
        if (oneShot) animCurrentFrame = 0;
    }

    void Animation::ChangeAnimation(int index, bool _oneShot = false)
    {
        ChangeAnimation(index, 1, _oneShot);
    }

    Animation::Animation(AssetID id)
    {
        animsCount = 0;
        animations = ResourceManager::GetInstance().GetModelAnimation(id, &animsCount);
        animIndex = 0;
    }
} // namespace sage
