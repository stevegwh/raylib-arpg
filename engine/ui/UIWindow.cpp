//
// Window family implementation. See UIWindow.hpp.
//

#include "UIWindow.hpp"

#include "../GameUiEngine.hpp"
#include "../ResourceManager.hpp"
#include "../Settings.hpp"
#include "Scrollbar.hpp"
#include "UIElements.hpp"

#include "raylib.h"
#include "raymath.h"

#include <cassert>
#include <cmath>
#include <queue>
#include <utility>

namespace sage
{
    void Window::InitLayout()
    {
        if (children.empty()) return;
        float availableWidth = rec.width - (padding.left + padding.right);
        if (overflowContingency == OverflowContingency::SCROLLBAR)
            availableWidth -= Scrollbar::GUTTER_WIDTH;
        const float availableHeight = rec.height - (padding.up + padding.down);
        const float startX = rec.x + padding.left;
        const float startY = rec.y + padding.up;

        std::vector<SizeRequest> requests;
        requests.reserve(children.size());
        for (const auto& p : children)
        {
            // Children of a Window are always Tables (added via CreateTable*).
            assert(dynamic_cast<Table*>(p.get()) != nullptr);
            auto* table = static_cast<Table*>(p.get());
            requests.push_back({table->autoSize, table->requestedHeight});
        }
        auto sizes = distributeAlong(availableHeight, requests);

        float currentY = startY;
        for (size_t i = 0; i < children.size(); ++i)
        {
            assert(dynamic_cast<Table*>(children[i].get()) != nullptr);
            auto* table = static_cast<Table*>(children[i].get());
            table->parent = this;
            table->rec = rec;

            const float panelHeight = sizes[i];
            table->rec.height = panelHeight;
            table->rec.y = currentY;
            table->rec.width = availableWidth;
            table->rec.x = startX;

            UpdateTextureDimensions();

            if (!table->children.empty()) table->InitLayout();

            currentY += panelHeight;
        }
    }

    void Window::SetPos(const float x, const float y)
    {
        const auto old = Vector2{rec.x, rec.y};
        rec = {x, y, rec.width, rec.height};
        ClampToScreen();
        const auto diff = Vector2Subtract(Vector2{rec.x, rec.y}, Vector2{old.x, old.y});

        std::queue<TableElement*> elementsToProcess;

        for (auto& panel : children)
        {
            elementsToProcess.push(panel.get());
        }

        while (!elementsToProcess.empty())
        {
            const auto current = elementsToProcess.front();
            elementsToProcess.pop();

            current->rec.x += diff.x;
            current->rec.y += diff.y;

            if (current->element.has_value())
            {
                current->element.value()->UpdateDimensions();
                continue; // Skip adding children since this is an end node
            }

            for (auto& child : current->children)
            {
                elementsToProcess.push(child.get());
            }
        }
    }

    void Window::ToggleHide()
    {
        hidden = !hidden;
        if (hidden)
        {
            onHide.Publish();
        }
        else
        {
            onShow.Publish();
        }
    }

    void Window::Show()
    {
        hidden = false;
        onShow.Publish();
    }

    void Window::Hide()
    {
        hidden = true;
        onHide.Publish();
    }

    bool Window::IsHidden() const
    {
        return hidden;
    }

    bool Window::IsMarkedForRemoval() const
    {
        return markForRemoval;
    }

    void Window::Remove()
    {
        hidden = true;
        markForRemoval = true;
        onHide.Publish();
    }

    void Window::SetOverflowContingency(const OverflowContingency contingency)
    {
        const bool widthChanged = (overflowContingency == OverflowContingency::SCROLLBAR) !=
                                  (contingency == OverflowContingency::SCROLLBAR);
        overflowContingency = contingency;
        if (contingency == OverflowContingency::SCROLLBAR && !scrollbar)
            scrollbar = std::make_unique<Scrollbar>();
        if (widthChanged && !children.empty()) InitLayout();
    }

    OverflowContingency Window::GetOverflowContingency() const
    {
        return overflowContingency;
    }

    Scrollbar* Window::GetScrollbar() const
    {
        return scrollbar.get();
    }

    void Window::Draw2D()
    {
        if (overflowContingency == OverflowContingency::SHRINK)
        {
            TableElement::Draw2D();
            return;
        }

        Rectangle content = rec;
        Rectangle gutter{};
        const bool hasScrollbar =
            overflowContingency == OverflowContingency::SCROLLBAR && scrollbar;
        if (hasScrollbar)
        {
            content.width -= Scrollbar::GUTTER_WIDTH;
            gutter = {
                rec.x + content.width, rec.y, Scrollbar::GUTTER_WIDTH, rec.height};
        }

        {
            const ScissorScope clip{content};
            TableElement::Draw2D();
        }

        if (hasScrollbar) scrollbar->DrawChrome(gutter);
    }

    void Window::Update()
    {
        if (overflowContingency == OverflowContingency::SCROLLBAR && scrollbar)
        {
            const Vector2 mousePos = GetMousePosition();
            if (!DropdownList::ActiveDropdownCapturesCursor(mousePos))
            {
                const Rectangle gutter = {
                    rec.x + rec.width - Scrollbar::GUTTER_WIDTH,
                    rec.y,
                    Scrollbar::GUTTER_WIDTH,
                    rec.height};
                scrollbar->HandleInput(rec, gutter);
            }
        }
        TableElement::Update();
    }

    void Window::FinalizeLayout()
    {
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
        for (const auto& child : children)
        {
            child->FinalizeLayout();
        }
        ScaleContents(settings);
    }

    void Window::OnWindowUpdate(Vector2 prev, Vector2 current)
    {
        ScaleContents(settings);
    }

    void Window::ScaleContents(Settings* _settings)
    {
        if (markForRemoval) return;

        Reset();

        rec = {
            settings->ScaleValueWidth(rec.x),
            settings->ScaleValueHeight(rec.y),
            settings->ScaleValueWidth(rec.width),
            settings->ScaleValueHeight(rec.height)};

        padding = {
            settings->ScaleValueHeight(padding.up),
            settings->ScaleValueHeight(padding.down),
            settings->ScaleValueWidth(padding.left),
            settings->ScaleValueWidth(padding.right)};

        UpdateTextureDimensions();

        for (const auto& child : children)
        {
            child->ScaleContents(settings);
        }
    }

    void Window::ClampToScreen()
    {
        const auto viewport = settings->GetViewPort();
        if (rec.x + rec.width > viewport.x)
        {
            rec.x = viewport.x - rec.width;
        }
        if (rec.y + rec.height > viewport.y + rec.height / 2)
        {
            rec.y = viewport.y - rec.height / 2;
        }
        if (rec.x < 0)
        {
            rec.x = 0;
        }
        if (rec.y < 0)
        {
            rec.y = 0;
        }
    }

    void Window::OnHoverStart()
    {
        UIElement::OnHoverStart();
    }

    Window::~Window()
    {
        windowUpdateSub.UnSubscribe();
    }

    Window::Window(Settings* _settings, const Padding _padding)
        : TableElement(nullptr, _padding), settings(_settings)
    {
    }

    Window::Window(
        Settings* _settings,
        const Texture& _tex,
        const TextureStretchMode _stretchMode,
        const float x,
        const float y,
        const float width,
        const float height,
        const Padding _padding)
        : TableElement(nullptr, x, y, width, height, _padding), settings(_settings)
    {
        tex = _tex;
        textureStretchMode = _stretchMode;
    }

    Window::Window(
        Settings* _settings,
        const float x,
        const float y,
        const float width,
        const float height,
        const Padding _padding)
        : TableElement(nullptr, x, y, width, height, _padding), settings(_settings)
    {
    }

    TableGrid* Window::CreateTableGrid(const int rows, const int cols, const float cellSpacing, Padding _padding)
    {
        children.push_back(std::make_unique<TableGrid>(this, _padding));
        const auto& table = dynamic_cast<TableGrid*>(children.back().get());
        table->cellSpacing = cellSpacing;
        // Create rows and cells with initial autoSize = true
        for (int i = 0; i < rows; ++i)
        {
            TableRow* row = table->CreateTableRow();
            for (int j = 0; j < cols; ++j)
            {
                row->CreateTableCell();
            }
        }
        InitLayout();
        return table;
    }

    Table* Window::CreateTable(Padding _padding)
    {
        children.push_back(std::make_unique<Table>(this, _padding));
        const auto& table = dynamic_cast<Table*>(children.back().get());
        InitLayout();
        return table;
    }

    Table* Window::CreateTable(const float _requestedHeight, const Padding _padding)
    {
        const auto table = CreateTable(_padding);
        table->autoSize = false;
        table->requestedHeight = _requestedHeight;
        InitLayout();
        return table;
    }

    void TooltipWindow::Remove()
    {
        hidden = true;
        markForRemoval = true;
    }

    void TooltipWindow::ScaleContents(Settings* _settings)
    {
        // Tooltips are constructed in viewport-coord by their factories (see
        // GameUiFactory's createTextTooltip / CreateWorldTooltip), so the rec and
        // padding are already at the correct viewport scale and no scaling is
        // applied here. Children of a tooltip are also laid out in viewport-coord
        // by per-Create InitLayouts (since they read their viewport-coord parent
        // rect at construction time), so we don't need to recurse either.
        //
        // Note: this means a tooltip won't re-fit if the OS window is resized
        // while it's visible. That's an accepted trade-off — tooltips are
        // transient and get recreated on the next hover at the new scale factor.
        if (markForRemoval) return;
        UpdateTextureDimensions();
    }

    TooltipWindow::~TooltipWindow()
    {
        if (parentWindowHideSub.IsActive())
        {
            parentWindowHideSub.UnSubscribe();
        }
    }

    TooltipWindow::TooltipWindow(
        Settings* _settings,
        Window* parentWindow,
        const Texture& _tex,
        TextureStretchMode _stretchMode,
        const float x,
        const float y,
        const float width,
        const float height,
        const Padding _padding)
        : Window(_settings, x, y, width, height, _padding)
    {
        if (parentWindow)
        {
            parentWindowHideSub = parentWindow->onHide.Subscribe([this]() { Remove(); });
        }
        tex = _tex;
        textureStretchMode = _stretchMode;
    }

    void WindowDocked::ScaleContents(Settings* _settings)
    {
        Reset();
        setAlignment();

        rec = {
            settings->ScaleValueWidth(rec.x),
            settings->ScaleValueHeight(rec.y),
            settings->ScaleValueWidth(rec.width),
            settings->ScaleValueHeight(rec.height)};

        padding = {
            settings->ScaleValueHeight(padding.up),
            settings->ScaleValueHeight(padding.down),
            settings->ScaleValueWidth(padding.left),
            settings->ScaleValueWidth(padding.right)};

        UpdateTextureDimensions();

        for (auto& child : children)
        {
            child->ScaleContents(settings);
        }
    }

    void WindowDocked::setAlignment()
    {
        float xOffset = 0;
        float yOffset = 0;

        // Calculate horizontal position
        switch (horiAlignment)
        {
        case HoriAlignment::LEFT:
            xOffset = 0;
            break;

        case HoriAlignment::CENTER:
            xOffset = (Settings::TARGET_SCREEN_WIDTH - rec.width) / 2;
            break;

        case HoriAlignment::RIGHT:
            xOffset = Settings::TARGET_SCREEN_WIDTH - rec.width;
            break;
        default:;
        }

        // Calculate vertical position
        switch (vertAlignment)
        {
        case VertAlignment::TOP:
            yOffset = 0;
            break;

        case VertAlignment::MIDDLE:
            yOffset = (Settings::TARGET_SCREEN_HEIGHT - rec.height) / 2;
            break;

        case VertAlignment::BOTTOM:
            yOffset = Settings::TARGET_SCREEN_HEIGHT - rec.height;
            break;
        }

        rec.x = xOffset + baseXOffset;
        rec.y = yOffset + baseYOffset;
    }

    WindowDocked::WindowDocked(
        Settings* _settings,
        const float _xOffset,
        const float _yOffset,
        const float _width,
        const float _height,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment,
        const Padding _padding)
        : Window(_settings, _padding),
          baseXOffset(_xOffset),
          baseYOffset(_yOffset),
          vertAlignment(_vertAlignment),
          horiAlignment(_horiAlignment)
    {
        rec.width = _width;
        rec.height = _height;
        setAlignment();
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
    }

    WindowDocked::WindowDocked(
        Settings* _settings,
        Texture _tex,
        const TextureStretchMode _textureStretchMode,
        const float _xOffset,
        const float _yOffset,
        const float _width,
        const float _height,
        const VertAlignment _vertAlignment,
        const HoriAlignment _horiAlignment,
        const Padding _padding)
        : Window(_settings, _padding),
          baseXOffset(_xOffset),
          baseYOffset(_yOffset),
          vertAlignment(_vertAlignment),
          horiAlignment(_horiAlignment)
    {
        tex = _tex;
        textureStretchMode = _textureStretchMode;
        rec.width = _width;
        rec.height = _height;
        setAlignment();
        unscaledDimensions.rec = rec;
        unscaledDimensions.padding = padding;
    }

    bool ErrorMessage::Finished() const
    {
        return GetTime() > initialTime + totalDisplayTime;
    }

    void ErrorMessage::Draw2D() const
    {
        const auto currentTime = GetTime();

        if (currentTime > initialTime + totalDisplayTime)
        {
            return;
        }

        const auto elapsedTime = currentTime - initialTime;
        unsigned char a = 255;

        if (elapsedTime > (totalDisplayTime - fadeOut))
        {
            const float fadeProgress = (elapsedTime - (totalDisplayTime - fadeOut)) / fadeOut;
            a = static_cast<unsigned char>((1.0f - fadeProgress) * 255);
        }

        const auto [width, height] = settings->GetViewPort();
        auto col = RAYWHITE;
        col.a = a;

        const auto scaledFontSize = settings->ScaleValueMaintainRatio(24);
        const auto textSize = MeasureText(msg.c_str(), scaledFontSize);

        DrawTextEx(
            font, msg.c_str(), Vector2{(width - textSize) / 2, height / 4}, scaledFontSize, fontSpacing, col);
    }

    ErrorMessage::ErrorMessage(Settings* _settings, std::string _msg)
        : settings(_settings),
          font(
              ResourceManager::GetInstance().FontLoad(
                  "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf")),
          fontSpacing(1.5),
          msg(std::move(_msg)),
          initialTime(GetTime())
    {
    }
} // namespace sage
