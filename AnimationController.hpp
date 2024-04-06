//
// Created by Steve Wheeler on 06/04/2024.
//

#include <memory>
#include <vector>

#include "AnimationState.hpp"

#pragma once

namespace sage
{
class AnimationController 
{
private:
    std::vector<std::unique_ptr<AnimationState>> states;
public:
    AnimationState* head;
    void Update();
    void Draw();
    void SetHead(AnimationState* state);
    void Add(std::unique_ptr<AnimationState> state);
    void Pop();
};
} // sage
