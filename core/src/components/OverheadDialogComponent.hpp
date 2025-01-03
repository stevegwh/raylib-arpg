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

    class OverheadDialogComponent
    {
        unsigned int currentIdx = 0;
        std::vector<std::string> text;
        double initialTime;
        float delay = 0;

      public:
        [[nodiscard]] bool IsFinished() const
        {
            return currentIdx >= text.size();
        }

        std::string GetText()
        {
            if (GetTime() >= initialTime + delay)
            {
                ++currentIdx;
                initialTime = GetTime();
            }
            if (currentIdx >= text.size())
            {
                return "";
            }
            return text.at(currentIdx);
        }

        void SetText(std::vector<std::string> _text, float _delay)
        {
            text = std::move(_text);
            delay = _delay;
        }

        explicit OverheadDialogComponent() : initialTime(GetTime())
        {
        }
    };

} // namespace sage
