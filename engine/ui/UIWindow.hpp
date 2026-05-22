//
// Top-level Window types: Window, TooltipWindow, WindowDocked, plus ErrorMessage.
// A Window is a TableElement that owns a tree of layout/elements and provides
// chrome (title bar drag, hide/show, scaling, removal).
//

#pragma once

#include "../Event.hpp"
#include "UIBase.hpp"
#include "UILayout.hpp"

#include "raylib.h"

#include <memory>
#include <string>

namespace sage
{
    class Scrollbar;
    struct Settings;

    // How a Window handles content that overflows its rect (vertically or
    // horizontally). Apply via Window::SetOverflowContingency.
    //  - TRUNCATE: clip the window rect with BeginScissorMode so out-of-bounds
    //    children/elements visually stop at the window edge.
    //  - SCROLLBAR: clip, plus the window owns a Scrollbar (see GetScrollbar())
    //    that reserves a gutter on the right and draws chrome. Consumers
    //    configure it with SetProviders(totalRows, visibleRows) and subscribe
    //    to onScrollChanged.
    //  - SHRINK: no clipping — assumes the contained elements will shrink to
    //    fit (this is the default behaviour today for TextBox shrinking).
    enum class OverflowContingency
    {
        TRUNCATE,
        SCROLLBAR,
        SHRINK
    };

    class Window : public TableElement
    {
        void ScaleContents(Settings* _settings) override;

      protected:
        bool hidden = false;
        bool markForRemoval = false;
        OverflowContingency overflowContingency = OverflowContingency::TRUNCATE;
        std::unique_ptr<Scrollbar> scrollbar;

      public:
        Event<> onHide;
        Event<> onShow;
        Subscription windowUpdateSub{};
        bool mouseHover = false;
        Settings* settings{}; // for screen width/height

        void SetPos(float x, float y) override;
        void FinalizeLayout() override;
        void OnWindowUpdate(Vector2 prev, Vector2 current);
        void ClampToScreen();
        void OnHoverStart() override;
        void Draw2D() override;
        void Update() override;
        TableGrid* CreateTableGrid(int rows, int cols, float cellSpacing = 0, Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void ToggleHide();
        void Show();
        void Hide();
        [[nodiscard]] bool IsHidden() const;
        [[nodiscard]] bool IsMarkedForRemoval() const;
        void SetOverflowContingency(OverflowContingency contingency);
        [[nodiscard]] OverflowContingency GetOverflowContingency() const;
        [[nodiscard]] Scrollbar* GetScrollbar() const;
        virtual void Remove();
        void InitLayout() override;
        ~Window() override;
        explicit Window(Settings* _settings, Padding _padding = {0, 0, 0, 0});
        Window(
            Settings* _settings,
            const Texture& _tex,
            TextureStretchMode _stretchMode,
            float x,
            float y,
            float width,
            float height,
            Padding _padding = {0, 0, 0, 0});
        Window(Settings* _settings, float x, float y, float width, float height, Padding _padding = {0, 0, 0, 0});

        friend class TitleBar;
        friend class GameUIEngine;
    };

    class TooltipWindow final : public Window
    {
        Subscription parentWindowHideSub{};

      public:
        void Remove() override;
        void ScaleContents(Settings* _settings) override;
        ~TooltipWindow() override;
        TooltipWindow(
            Settings* _settings,
            Window* parentWindow,
            const Texture& _tex,
            TextureStretchMode _stretchMode,
            float x,
            float y,
            float width,
            float height,
            Padding _padding = {0, 0, 0, 0});
        friend class GameUIEngine;
    };

    class WindowDocked final : public Window
    {
        float baseXOffset = 0;
        float baseYOffset = 0;
        void setAlignment();
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;

      public:
        void ScaleContents(Settings* _settings) override;
        WindowDocked(
            Settings* _settings,
            float _xOffset,
            float _yOffset,
            float _width,
            float _height,
            VertAlignment _vertAlignment,
            HoriAlignment _horiAlignment,
            Padding _padding = {0, 0, 0, 0});

        WindowDocked(
            Settings* _settings,
            Texture _tex,
            TextureStretchMode _textureStretchMode,
            float _xOffset,
            float _yOffset,
            float _width,
            float _height,
            VertAlignment _vertAlignment,
            HoriAlignment _horiAlignment,
            Padding _padding = {0, 0, 0, 0});

        friend class GameUIEngine;
    };

    class ErrorMessage
    {
        Settings* settings;
        Font font{};
        float fontSpacing;
        std::string msg;
        double initialTime;
        float totalDisplayTime = 3.0f;
        float fadeOut = 1.0f;

      public:
        [[nodiscard]] bool Finished() const;
        void Draw2D() const;

        explicit ErrorMessage(Settings* _settings, std::string _msg);
    };
} // namespace sage
