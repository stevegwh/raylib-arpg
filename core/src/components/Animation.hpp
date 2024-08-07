//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

#include "ResourceManager.hpp"

#include "raylib.h"
#include <entt/entt.hpp>

#include <unordered_map>

namespace sage
{
    enum class AnimationEnum
    {
        IDLE,
        DEATH,
        AUTOATTACK,
        MOVE,
        TALK,
        SPIN
    };

    struct Animation
    {
        std::unordered_map<AnimationEnum, int> animationMap;
        ModelAnimation* animations;
        Model* model;
        unsigned int animIndex = 0;
        unsigned int animCurrentFrame = 0;
        unsigned int animLastFrame = 0;
        int animsCount;
        int animSpeed = 1;
        bool oneShot = false;
        entt::sigh<void(entt::entity)> onAnimationEnd{};
        entt::sigh<void(entt::entity)> onAnimationStart{};

        Animation(const char* _modelPath, Model* _model) : model(_model)
        {
            animsCount = 0;
            animations = ResourceManager::ModelAnimationLoad(_modelPath, &animsCount);
            animIndex = 0;
        }

        Animation(const Animation&) = delete;
        Animation& operator=(const Animation&) = delete;

        bool ChangeAnimationByEnum(
            AnimationEnum animEnum, int _animSpeed, bool _oneShot = false)
        {
            if (!animationMap.contains(animEnum)) return false;
            ChangeAnimation(animationMap.at(animEnum), _animSpeed, _oneShot);
            return true;
        }

        bool ChangeAnimationByEnum(AnimationEnum animEnum, bool _oneShot = false)
        {
            return ChangeAnimationByEnum(animEnum, 1, _oneShot);
        }

        void ChangeAnimation(int index, int _animSpeed, bool _oneShot = false)
        {
            animSpeed = _animSpeed;
            animIndex = index;
            oneShot = _oneShot;
            if (oneShot) animCurrentFrame = 0;
        }

        void ChangeAnimation(int index, bool _oneShot = false)
        {
            ChangeAnimation(index, 1, _oneShot);
        }
    };
} // namespace sage
