//
// Created by Steve Wheeler on 06/04/2024.
//

#include "AnimationController.hpp"

namespace sage
{

void AnimationController::SetHead(AnimationState* state)
{
    if (head != nullptr)
    {
        state->next = head;
    }
    head = state;

}

void AnimationController::Pop()
{
    if (head == nullptr || head->next == nullptr) return; // NOTE: Always leaves 1 state in stack.
    head = head->next;
    //free(tmp);
}

void AnimationController::Update()
{
    if (!head->CheckCondition())
    {
        Pop();
        return;
    }
    head->Update();
}

void AnimationController::Draw()
{
    head->Draw();
}

void AnimationController::Add(std::unique_ptr<AnimationState> state)
{
    states.push_back(std::move(state));
}
} // sage