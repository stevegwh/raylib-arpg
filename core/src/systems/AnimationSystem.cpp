//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationSystem.hpp"
#include "../Application.hpp"

namespace sage
{

void AnimationSystem::Update()
{
    const auto& view = registry->view<Animation, Renderable>();
    view.each([](auto& a, auto& r) {
        ModelAnimation anim = a.animations[a.animIndex];
        if (a.oneShot && a.animCurrentFrame + 1 >= anim.frameCount) return;
        a.animCurrentFrame = (a.animCurrentFrame + 1) % anim.frameCount;
        UpdateModelAnimation(r.model, anim, a.animCurrentFrame);
    });
}

void AnimationSystem::Draw()
{

}
AnimationSystem::AnimationSystem(entt::registry *_registry)
    : BaseSystem<Animation>(_registry)
{

}
} // sage