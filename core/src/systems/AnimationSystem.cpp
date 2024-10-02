//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationSystem.hpp"
#include "components/Animation.hpp"
#include "components/Renderable.hpp"
#include "components/WeaponComponent.hpp"

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
                animation.onAnimationStart.publish(entity);
            }

            bool finalFrame = animData.currentFrame + animData.speed >= anim.frameCount;
            animData.lastFrame = animData.currentFrame;
            animData.currentFrame = (animData.currentFrame + animData.speed) % anim.frameCount;
            renderable.GetModel()->UpdateAnimation(anim, animData.currentFrame);

            if (finalFrame) // Must be at end, as end of death animations can result in entities being destroyed
            {
                animation.onAnimationEnd.publish(entity);
                if (animation.oneShotMode)
                {
                    animation.RestoreAfterOneShot();
                }
            }
        }

        // Update the positions of any weapons in the scene post animation update
        for (auto view = registry->view<WeaponComponent>(); auto entity : view)
        {
            auto& weapon = registry->get<WeaponComponent>(entity);
            auto& weaponRend = registry->get<Renderable>(entity);
            auto& model = registry->get<Renderable>(weapon.owner).GetModel()->GetRlModel();
            auto boneId = GetBoneIdByName(model.bones, model.boneCount, weapon.parentBoneName.c_str());
            assert(boneId >= 0);
            auto* matrices = model.meshes[0].boneMatrices;
            auto mat = matrices[boneId];
            mat = MatrixMultiply(weapon.parentSocket, mat);
            mat = MatrixMultiply(mat, weaponRend.initialTransform);
            weaponRend.GetModel()->SetTransform(mat);
        }
    }

    void AnimationSystem::Draw()
    {
    }

    AnimationSystem::AnimationSystem(entt::registry* _registry) : BaseSystem(_registry)
    {
    }
} // namespace sage
