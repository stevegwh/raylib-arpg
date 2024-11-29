//
// Created by Steve Wheeler on 29/11/2024.
//

#pragma once

namespace sage
{
    class Quest
    {
        bool completed = false;

      public:
        [[nodiscard]] bool IsComplete();
    };

} // namespace sage
