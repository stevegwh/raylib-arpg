//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include <memory>


namespace sage
{
class AnimationState
{
protected:
    AnimationState() = default;
public:
    virtual ~AnimationState();
    virtual bool CheckCondition() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    AnimationState* next{};
    //ModelAnimation* animation;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    int animsCount = 0;
};
} // sage
