//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include "Component.hpp"

#include <memory>

#include "EventManager.hpp"

namespace sage
{
struct Animation
{
    std::unique_ptr<EventManager> eventManager;
    ModelAnimation* animations;
    Model* model;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    int animsCount;
    
    Animation(const char* _modelPath, Model* _model) :
    model(_model), eventManager(std::make_unique<EventManager>())
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
};
}