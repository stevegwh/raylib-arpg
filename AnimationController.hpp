//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include "raylib.h"

#include "Entity.hpp"

namespace sage
{

class AnimationController
{
    EntityID entityID;
    const Transform* transform;
    ModelAnimation* animation;
    Model* model;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    int animsCount;
protected:
    void changeAnimation(int index);
public:
    ~AnimationController();
    void Update();
    void Draw();
    
    AnimationController(EntityID _entityId , const char* _modelPath, Model* _model, const Transform* _transform) : 
    entityID(_entityId), model(_model), transform(_transform)
    {
        animsCount = 0;
        animation = LoadModelAnimations(_modelPath, &animsCount);
        animIndex = 0;
    }

};

} // sage
