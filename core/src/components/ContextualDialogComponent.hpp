//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace sage
{

    class ContextualDialogComponent
    {
        entt::entity self;
        unsigned int currentIdx = 0;
        std::vector<std::string> text;
        double initialTime;
        float delay = 0;

      public:
        entt::sigh<void(entt::entity)> onDialogFinished;

        std::string GetText()
        {
            if (GetTime() >= initialTime + delay)
            {
                ++currentIdx;
                initialTime = GetTime();
            }
            if (currentIdx >= text.size())
            {
                onDialogFinished.publish(self);
                return "";
            }
            return text.at(currentIdx);
        }

        void SetText(std::vector<std::string> _text, float _delay)
        {
            text = std::move(_text);
            delay = _delay;
        }

        explicit ContextualDialogComponent(entt::entity _self) : self(_self), initialTime(GetTime())
        {
        }
    };

} // namespace sage
