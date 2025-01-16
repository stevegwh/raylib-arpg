//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

#include <string>
#include <utility>
#include <vector>

namespace sage
{

    class OverheadDialogComponent
    {
        unsigned int currentIdx = 0;
        std::vector<std::pair<std::string, std::function<bool()>>> text;
        double initialTime;
        float delay = 0;
        bool loop = false;

      public:
        [[nodiscard]] bool IsFinished() const
        {
            return currentIdx >= text.size();
        }

        void Reset()
        {
            currentIdx = 0;
        }

        std::string GetText()
        {
            if (GetTime() >= initialTime + delay)
            {
                ++currentIdx;

                if (currentIdx < text.size())
                {
                    const auto& [txt, condition] = text.at(currentIdx);
                    if (condition())
                    {
                        initialTime = GetTime();
                        return txt;
                    }
                    else
                    {
                        return GetText();
                    }
                }

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
            const auto& [txt, _] = text.at(currentIdx);
            return txt;
        }

        void SetText(std::vector<std::pair<std::string, std::function<bool()>>> _text, float _delay)
        {
            text = std::move(_text);
            delay = _delay;
        }

        explicit OverheadDialogComponent(bool _loop = false) : initialTime(GetTime()), loop(_loop)
        {
        }
    };

} // namespace sage
