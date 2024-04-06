//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationController.hpp"
#include "GameManager.hpp"

namespace sage
{

void AnimationController::changeAnimation(int index)
{
    animIndex = index;
}

void AnimationController::Update()
{
    ModelAnimation anim = animation[animIndex];
    animCurrentFrame = (animCurrentFrame + 1) % anim.frameCount;
    UpdateModelAnimation(ECS->renderSystem->GetComponent(entityID)->model, anim, animCurrentFrame);
}

void AnimationController::Draw()
{

}

AnimationController::~AnimationController()
{
    UnloadModelAnimations(animation, animsCount);
}
} // sage