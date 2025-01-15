//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

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
        bool loop = false;

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
                if (!loop)
                {
                    return "";
                }
                currentIdx = 0;
            }
            return text.at(currentIdx);
        }

        void SetText(std::vector<std::string> _text, float _delay)
        {
            text = std::move(_text);
            delay = _delay;
        }

        explicit OverheadDialogComponent(bool _loop = false) : initialTime(GetTime()), loop(_loop)
        {
        }
    };

} // namespace sage
