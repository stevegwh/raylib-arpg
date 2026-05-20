#include "Scrollbar.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace sage
{
    void Scrollbar::SetProviders(
        std::function<std::size_t()> totalRows, std::function<std::size_t()> visibleRows)
    {
        totalRowsProvider = std::move(totalRows);
        visibleRowsProvider = std::move(visibleRows);
    }

    std::size_t Scrollbar::TotalRows() const
    {
        return totalRowsProvider ? totalRowsProvider() : 0;
    }

    std::size_t Scrollbar::VisibleRows() const
    {
        return visibleRowsProvider ? visibleRowsProvider() : 0;
    }

    bool Scrollbar::HasOverflow() const
    {
        return TotalRows() > VisibleRows();
    }

    std::size_t Scrollbar::ScrollOffset() const
    {
        return scrollOffset;
    }

    void Scrollbar::Scroll(const int signedDelta)
    {
        const std::size_t total = TotalRows();
        const std::size_t visible = VisibleRows();
        const std::size_t maxOffset = total > visible ? total - visible : 0;

        const std::size_t previous = scrollOffset;
        if (signedDelta < 0)
        {
            const auto positive = static_cast<std::size_t>(-signedDelta);
            scrollOffset = positive > scrollOffset ? 0 : scrollOffset - positive;
        }
        else if (signedDelta > 0)
        {
            scrollOffset = std::min(maxOffset, scrollOffset + static_cast<std::size_t>(signedDelta));
        }

        if (scrollOffset != previous) onScrollChanged.Publish();
    }

    void Scrollbar::SetScrollOffset(const std::size_t offset)
    {
        const std::size_t total = TotalRows();
        const std::size_t visible = VisibleRows();
        const std::size_t maxOffset = total > visible ? total - visible : 0;

        const std::size_t previous = scrollOffset;
        scrollOffset = std::min(offset, maxOffset);
        if (scrollOffset != previous) onScrollChanged.Publish();
    }

    void Scrollbar::ClampOffset()
    {
        const std::size_t total = TotalRows();
        const std::size_t visible = VisibleRows();
        const std::size_t maxOffset = total > visible ? total - visible : 0;

        const std::size_t previous = scrollOffset;
        scrollOffset = std::min(scrollOffset, maxOffset);
        if (scrollOffset != previous) onScrollChanged.Publish();
    }

    void Scrollbar::HandleInput(const Rectangle& windowRect, const Rectangle& gutterRect)
    {
        if (!HasOverflow()) return;
        const Vector2 mousePos = GetMousePosition();

        if (CheckCollisionPointRec(mousePos, windowRect))
        {
            const float wheel = GetMouseWheelMove();
            if (wheel > 0.0f)
            {
                Scroll(-static_cast<int>(std::ceil(wheel)));
            }
            else if (wheel < 0.0f)
            {
                Scroll(static_cast<int>(std::ceil(-wheel)));
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            const Rectangle upRec = {gutterRect.x, gutterRect.y, gutterRect.width, BUTTON_HEIGHT};
            const Rectangle downRec = {
                gutterRect.x,
                gutterRect.y + gutterRect.height - BUTTON_HEIGHT,
                gutterRect.width,
                BUTTON_HEIGHT};
            if (CheckCollisionPointRec(mousePos, upRec)) Scroll(-1);
            else if (CheckCollisionPointRec(mousePos, downRec)) Scroll(1);
        }
    }

    void Scrollbar::DrawChrome(const Rectangle& gutterRect) const
    {
        if (!HasOverflow()) return;

        const Color buttonBg{233, 238, 246, 255};
        const Color buttonOutline{151, 164, 184, 255};
        const Color trackBg{24, 26, 30, 255};
        const Color trackOutline{69, 74, 84, 255};
        const Color thumbColor{139, 148, 164, 255};

        const Rectangle upRec = {gutterRect.x, gutterRect.y, gutterRect.width, BUTTON_HEIGHT};
        const Rectangle downRec = {
            gutterRect.x,
            gutterRect.y + gutterRect.height - BUTTON_HEIGHT,
            gutterRect.width,
            BUTTON_HEIGHT};
        const Rectangle trackRec = {
            gutterRect.x,
            gutterRect.y + BUTTON_HEIGHT,
            gutterRect.width,
            gutterRect.height - 2.0f * BUTTON_HEIGHT};

        DrawRectangleRec(upRec, buttonBg);
        DrawRectangleLinesEx(upRec, 1.0f, buttonOutline);
        DrawRectangleRec(downRec, buttonBg);
        DrawRectangleLinesEx(downRec, 1.0f, buttonOutline);

        constexpr int arrowFontSize = 10;
        const int upTextWidth = MeasureText("^", arrowFontSize);
        const int downTextWidth = MeasureText("v", arrowFontSize);
        DrawText(
            "^",
            static_cast<int>(upRec.x + (upRec.width - static_cast<float>(upTextWidth)) * 0.5f),
            static_cast<int>(upRec.y + (upRec.height - static_cast<float>(arrowFontSize)) * 0.5f),
            arrowFontSize,
            BLACK);
        DrawText(
            "v",
            static_cast<int>(downRec.x + (downRec.width - static_cast<float>(downTextWidth)) * 0.5f),
            static_cast<int>(
                downRec.y + (downRec.height - static_cast<float>(arrowFontSize)) * 0.5f),
            arrowFontSize,
            BLACK);

        DrawRectangleRec(trackRec, trackBg);
        DrawRectangleLinesEx(trackRec, 1.0f, trackOutline);

        const float visibleCount = static_cast<float>(VisibleRows());
        const float totalCount = static_cast<float>(TotalRows());
        if (totalCount <= 0.0f || visibleCount <= 0.0f) return;

        const float thumbHeight = totalCount <= visibleCount
                                      ? trackRec.height
                                      : std::max(18.0f, trackRec.height * (visibleCount / totalCount));
        const std::size_t maxOffset = TotalRows() > VisibleRows() ? TotalRows() - VisibleRows() : 0;
        const float scrollRatio =
            maxOffset == 0
                ? 0.0f
                : static_cast<float>(scrollOffset) / static_cast<float>(maxOffset);
        const float thumbY = trackRec.y + (trackRec.height - thumbHeight) * scrollRatio;
        DrawRectangleRec(
            {trackRec.x + 2.0f, thumbY + 2.0f, trackRec.width - 4.0f, thumbHeight - 4.0f},
            thumbColor);
    }
} // namespace sage
