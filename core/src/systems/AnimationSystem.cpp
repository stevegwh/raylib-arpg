//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationSystem.hpp"
#include "components/Animation.hpp"
#include "components/Renderable.hpp"
#include "components/WeaponComponent.hpp"
#include "Event.hpp"

namespace sage
{

    void AnimationSystem::Update() const
    {
        for (const auto& view = registry->view<Animation, Renderable>(); auto& entity : view)
        {
            auto& animation = registry->get<Animation>(entity);
            auto& renderable = registry->get<Renderable>(entity);
            auto& animData = animation.current;
            const ModelAnimation& anim = animation.animations[animData.index];

            if (animData.currentFrame == 0 || animData.currentFrame < animData.lastFrame)
            {
                animation.onAnimationStart.Publish(entity);
            }

            bool finalFrame = animData.currentFrame + animData.speed >= anim.frameCount;
            animData.lastFrame = animData.currentFrame;
            animData.currentFrame = (animData.currentFrame + animData.speed) % anim.frameCount;
            renderable.GetModel()->UpdateAnimation(anim, animData.currentFrame);

            if (finalFrame) // Must be at end, as end of death animations can result in entities being destroyed
            {
                animation.onAnimationEnd.Publish(entity);
                if (animation.oneShotMode)
                {
                    animation.RestoreAfterOneShot();
                }
            }
            animation.onAnimationUpdated.Publish(entity);
        }
    }

    void AnimationSystem::Draw()
    {
    }

    AnimationSystem::AnimationSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
    }
} // namespace sage
