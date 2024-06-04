//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationSystem.hpp"
#include "../components/Renderable.hpp"

namespace sage
{

void AnimationSystem::Update()
{
    const auto& view = registry->view<Animation, Renderable>();
    for (auto& entity : view)
    {
        auto& a = registry->get<Animation>(entity);
        auto& r = registry->get<Renderable>(entity);
        ModelAnimation anim = a.animations[a.animIndex];
        
        if (a.animCurrentFrame == 0)
        {
            a.onAnimationStart.publish(entity);
        }
        
        if (a.animCurrentFrame + 1 >= anim.frameCount)
        {
            a.onAnimationEnd.publish(entity);
            if (a.oneShot) return;
        }
        
        a.animCurrentFrame = (a.animCurrentFrame + 1) % anim.frameCount;
        UpdateModelAnimation(r.model, anim, a.animCurrentFrame);
    };
}

void AnimationSystem::Draw()
{

}
AnimationSystem::AnimationSystem(entt::registry *_registry)
    : BaseSystem<Animation>(_registry)
{

}
} // sage