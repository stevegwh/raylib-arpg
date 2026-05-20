//
// Scrollbar: a self-contained vertical scrollbar drawn as Window chrome.
// Owned by a Window that has set OverflowContingency::SCROLLBAR. Consumers
// supply two size providers (total rows, visible rows) and subscribe to
// onScrollChanged to know when to refresh their content.
//

#pragma once

#include "../Event.hpp"

#include "raylib.h"

#include <cstddef>
#include <functional>

namespace sage
{
    class Scrollbar
    {
        std::size_t scrollOffset = 0;
        std::function<std::size_t()> totalRowsProvider;
        std::function<std::size_t()> visibleRowsProvider;

      public:
        static constexpr float GUTTER_WIDTH = 16.0f;
        static constexpr float BUTTON_HEIGHT = 12.0f;

        Event<> onScrollChanged;

        void SetProviders(
            std::function<std::size_t()> totalRows, std::function<std::size_t()> visibleRows);

        void Scroll(int signedDelta);
        void ClampOffset();

        [[nodiscard]] std::size_t ScrollOffset() const;
        [[nodiscard]] std::size_t TotalRows() const;
        [[nodiscard]] std::size_t VisibleRows() const;
        [[nodiscard]] bool HasOverflow() const;

        void HandleInput(const Rectangle& windowRect, const Rectangle& gutterRect);
        void DrawChrome(const Rectangle& gutterRect) const;
    };
} // namespace sage
