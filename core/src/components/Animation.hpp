//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include <memory>

namespace sage
{
struct Animation
{
    ModelAnimation* animations;
    Model* model;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    int animsCount;
    
    Animation(const char* _modelPath, Model* _model) :
    model(_model)
    {
        animsCount = 0;
        animations = LoadModelAnimations(_modelPath, &animsCount);
        animIndex = 0;
    }
    
    ~Animation()
    {
        UnloadModelAnimations(animations, animsCount);
    }

    void ChangeAnimation(int index)
    {
        animIndex = index;
    }
    
    // temp
    void SetAnimation3(entt::entity t)
    {
        animIndex = 3;
    }
    void SetAnimation0(entt::entity t)
    {
        animIndex = 0;
    }
};
}