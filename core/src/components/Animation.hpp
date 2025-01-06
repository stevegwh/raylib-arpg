//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include "ResourceManager.hpp"

#include "Event.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

#include <memory>
#include <unordered_map>

namespace sage
{
    enum class AnimationEnum
    {
        IDLE,
        IDLE2,
        DEATH,
        AUTOATTACK,
        WALK,
        TALK,
        SPIN,
        SLASH,
        RUN,
        SPELLCAST_FWD,
        SPELLCAST_UP,
        ROLL
    };

    struct AnimationParams
    {
        AnimationEnum animEnum = AnimationEnum::IDLE;
        int animSpeed = 1;
        bool oneShot = false;
        float animationDelay = 0;
    };

    // TODO: Use a timer for animation delay
    struct Animation
    {
        struct AnimData
        {
            unsigned int index = 0;
            unsigned int currentFrame = 0;
            unsigned int lastFrame = 0;
            int speed = 1;
        };

        std::unordered_map<AnimationEnum, int> animationMap;
        ModelAnimation* animations;
        int animsCount;

        bool oneShotMode = false;
        AnimData current{};

        Event<entt::entity> onAnimationEnd{};
        Event<entt::entity> onAnimationStart{};
        Event<entt::entity> onAnimationUpdated{};

        void ChangeAnimationByParams(AnimationParams params);
        void ChangeAnimationByEnum(AnimationEnum animEnum, int _animSpeed);
        void ChangeAnimationByEnum(AnimationEnum animEnum);
        void ChangeAnimation(int index);
        void ChangeAnimation(int index, int _animSpeed);

        void PlayOneShot(AnimationEnum animEnum, int _animSpeed);
        void PlayOneShot(int index, int _animSpeed);
        void RestoreAfterOneShot();

        Animation(const Animation&) = delete;
        Animation& operator=(const Animation&) = delete;
        explicit Animation(AssetID id);

      private:
        AnimData prev{};
    };
} // namespace sage
