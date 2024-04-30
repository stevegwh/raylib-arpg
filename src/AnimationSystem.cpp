//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationSystem.hpp"
#include "GameManager.hpp"

namespace sage
{

void AnimationSystem::Update()
{
    for (const auto& c: components) 
    {
        ModelAnimation anim = c.second->animations[c.second->animIndex];
        c.second->animCurrentFrame = (c.second->animCurrentFrame + 1) % anim.frameCount;
        UpdateModelAnimation(ECS->renderSystem->GetComponent(c.second->entityId)->model, anim, c.second->animCurrentFrame);
    }
}

void AnimationSystem::Draw()
{

}
} // sage