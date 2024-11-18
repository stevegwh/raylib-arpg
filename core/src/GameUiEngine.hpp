//
// Created by steve on 02/10/2024.
//

#pragma once

#include "Settings.hpp"

#include "components/DialogComponent.hpp"
#include "raylib.h"
#include "ResourceManager.hpp"
#include "Timer.hpp"

#include <entt/entt.hpp>
#include <optional>
#include <vector>

namespace sage
{
    class TooltipWindow;
    class GameData;
    class GameUIEngine;
    struct Settings;
    class UserInput;
    class Cursor;
    struct ItemComponent;
    class EquipmentComponent;
    enum class EquipmentSlotName;
    class PartySystem;
    class Window;
    class Panel;
    class Table;
    class TableRow;
    class TableCell;
    class UIState;
    class PlayerAbilitySystem;
    class ControllableActorSystem;

    enum class TextureStretchMode
    {
        NONE,
        SCALE,
    };

    struct Dimensions
    {
        float width = 0;
        float height = 0;
    };

    enum class HoriAlignment
    {
        LEFT,
        RIGHT,
        CENTER,
        WINDOW_CENTER
    };

    enum class VertAlignment
    {
        TOP,
        MIDDLE,
        BOTTOM
    };

    struct Padding
    {
        float up = 0;
        float down = 0;
        float left = 0;
        float right = 0;
    };

    struct Margin
    {
        float up = 0;
        float down = 0;
        float left = 0;
        float right = 0;
    };

    class UIElement
    {
      public:
        Rectangle rec{};
        virtual void OnIdleStart(){};
        virtual void OnIdleStop(){};
        virtual void OnHoverStart();
        virtual void OnHoverStop();
        virtual ~UIElement() = default;
        UIElement() = default;
    };

    template <typename Child, typename Parent>
    class TableElement : public UIElement
    {
      protected:
        struct OriginalDimensions
        {
            Rectangle rec{};
            Padding padding{};
        };

        OriginalDimensions ogDimensions{};
        Padding padding;
        TextureStretchMode textureStretchMode = TextureStretchMode::NONE;

        // Sets padding by pixel value (screen scaling is applied)
        void setPadding(const Padding& _padding)
        {
            ogDimensions.padding = _padding;
            padding = _padding;
        }

      public:
        Parent* parent{};
        Child children;

        std::optional<Texture> tex{};
        std::optional<NPatchInfo> nPatchInfo{};

        void SetTexture(const Texture& _tex, TextureStretchMode _stretchMode)
        {
            tex = _tex;
            textureStretchMode = _stretchMode;
            UpdateTextureDimensions();
        }

        void UpdateTextureDimensions()
        {
            if (!tex.has_value()) return;
            if (textureStretchMode == TextureStretchMode::SCALE)
            {
                tex->width = rec.width;
                tex->height = rec.height;
            }
        }

        virtual void UpdateChildren() = 0;
        virtual void DrawDebug2D() = 0;
        virtual void Draw2D()
        {
            if (tex.has_value())
            {
                if (nPatchInfo.has_value())
                {
                    DrawTextureNPatch(
                        tex.value(),
                        nPatchInfo.value(),
                        rec,
                        {0.0f, 0.0f},
                        0.0f,
                        WHITE); // Use {0.0f, 0.0f} for origin
                }
                else
                {
                    // TODO: Copy Window's desktop image for scaling (scale to fit, stretch, etc).
                    // DrawTexture(tex.value(), rec.x, rec.y, WHITE);
                    DrawTexture(tex.value(), rec.x, rec.y, WHITE);
                }
            }
        }

        // Returns pixel value of padding
        [[nodiscard]] const Padding& GetPadding() const
        {
            return padding;
        }

        [[nodiscard]] Window* GetWindow()
        {

            TableElement* current = this;
            while (current->parent != nullptr)
            {
                current = reinterpret_cast<TableElement*>(current->parent);
            }

            return reinterpret_cast<Window*>(current);
        }

        TableElement(Parent* _parent, float x, float y, float width, float height, Padding _padding)
            : padding(_padding), parent(_parent)
        {
            rec = {x, y, width, height};
            ogDimensions = {rec, padding};
        }

        TableElement(Parent* _parent, Padding _padding) : padding(_padding), parent(_parent)
        {
            ogDimensions = {rec, padding};
        }

        TableElement(const TableElement&) = default;
        TableElement(TableElement&&) noexcept = default;
        TableElement& operator=(const TableElement&) = default;
        TableElement& operator=(TableElement&&) noexcept = default;
        ~TableElement() override = default;
    };

    class CellElement : public UIElement
    {
      protected:
        entt::sigh<void()> onMouseClicked;
        Vector2 dragOffset{};
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;

      public:
        TableCell* parent{};
        GameUIEngine* engine{};

        std::unique_ptr<UIState> state;
        Texture tex{};
        bool canReceiveDragDrops = false;
        bool draggable = false;
        bool beingDragged = false;
        bool stateLocked = false;
        float dragDelayTime = 0.1;

        virtual void OnClick();
        virtual void HoverUpdate();
        virtual void OnDragStart();
        virtual void DragUpdate(){};
        virtual void DragDraw(){};
        virtual void OnDrop(CellElement* receiver);
        virtual void ReceiveDrop(CellElement* droppedElement);
        void ChangeState(std::unique_ptr<UIState> newState);
        virtual void UpdateDimensions();
        virtual void Draw2D() = 0;
        explicit CellElement(
            GameUIEngine* _engine,
            TableCell* _parent,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
        ~CellElement() override = default;
        friend class TableCell;
    };

    class TextBox : public CellElement
    {
      public:
        enum class OverflowBehaviour
        {
            SHRINK_TO_FIT,
            WORD_WRAP
        };

        struct FontInfo
        {
            float baseFontSize;
            float fontSize;
            float fontSpacing;
            Font font;
            static constexpr float minFontSize = 16.0f;
            static constexpr float maxFontSize = 72.0f;
            OverflowBehaviour overflowBehaviour;
            FontInfo()
                : baseFontSize(16),
                  fontSize(16),
                  fontSpacing(1.5),
                  font(ResourceManager::GetInstance().FontLoad(
                      "resources/fonts/LibreBaskerville/LibreBaskerville-Bold.ttf")),
                  overflowBehaviour(OverflowBehaviour::SHRINK_TO_FIT)
            {
            }
        };
        [[nodiscard]] const std::string& GetContent() const;
        void SetContent(const std::string& _content);
        [[nodiscard]] Font GetFont() const;
        void UpdateFontScaling();
        void UpdateDimensions() override;
        void Draw2D() override;
        ~TextBox() override = default;

        TextBox(
            GameUIEngine* _engine,
            TableCell* _parent,
            FontInfo _fontInfo = FontInfo(),
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);

      protected:
        Shader sdfShader;
        FontInfo fontInfo;
        std::string content;
    };

    class DialogOption : public TextBox
    {
        const dialog::Option& option;
        bool drawHighlight = false;

      public:
        void OnHoverStart() override;
        void OnHoverStop() override;
        void Draw2D() override;
        void OnClick() override;
        DialogOption(
            GameUIEngine* _engine,
            TableCell* _parent,
            const dialog::Option& _option,
            const FontInfo& _fontInfo,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
    };

    class TitleBar final : public TextBox
    {

      public:
        std::optional<Window*> draggedWindow;
        ~TitleBar() override = default;
        void OnDragStart() override;
        void DragUpdate() override;
        void OnDrop(CellElement* droppedElement) override;
        TitleBar(GameUIEngine* _engine, TableCell* _parent, const FontInfo& _fontInfo);
    };

    class ImageBox : public CellElement
    {
      public:
        enum class OverflowBehaviour
        {
            SHRINK_TO_FIT,
            SHRINK_ROW_TO_FIT
        };
        void OnIdleStart() override;
        void OnHoverStart() override;
        void OnHoverStop() override;
        void OnDragStart() override;
        void DragDraw() override;
        void OnDrop(CellElement* droppedElement) override;
        // void SetOverflowBehaviour(OverflowBehaviour _behaviour);
        void SetHoverShader();
        void SetGrayscale();
        void RemoveShader();
        void UpdateDimensions() override;
        void Draw2D() override;
        ImageBox(
            GameUIEngine* _engine,
            TableCell* _parent,
            const Texture& _tex,
            OverflowBehaviour _behaviour = OverflowBehaviour::SHRINK_TO_FIT,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
        ImageBox(
            GameUIEngine* _engine,
            TableCell* _parent,
            OverflowBehaviour _behaviour = OverflowBehaviour::SHRINK_TO_FIT,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
        ~ImageBox() override = default;

      protected:
        double hoverTimer = 0;
        float hoverTimerThreshold = 0.8;
        std::optional<TooltipWindow*> tooltipWindow;
        OverflowBehaviour overflowBehaviour = OverflowBehaviour::SHRINK_TO_FIT;
        std::optional<Shader> shader;
        virtual void updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space);

      private:
        [[nodiscard]] Dimensions calculateAvailableSpace() const;
        [[nodiscard]] float calculateAspectRatio() const;
        [[nodiscard]] Dimensions calculateInitialDimensions(const Dimensions& space) const;
        [[nodiscard]] Vector2 calculateAlignmentOffset(
            const Dimensions& dimensions, const Dimensions& space) const;
        void shrinkRowToFit() const;
        [[nodiscard]] Dimensions handleOverflow(const Dimensions& dimensions, const Dimensions& space) const;
    };

    class EquipmentCharacterPreview : public ImageBox
    {

      public:
        void UpdateDimensions() override;
        void RetrieveInfo();
        void Draw2D() override;
        EquipmentCharacterPreview(
            GameUIEngine* _engine,
            TableCell* _parent,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
    };

    class PartyMemberPortrait : public ImageBox
    {
        unsigned int memberNumber{};
        Texture portraitBgTex{};

      public:
        void RetrieveInfo();
        void ReceiveDrop(CellElement* droppedElement) override;
        void OnClick() override;
        void Draw2D() override;
        PartyMemberPortrait(GameUIEngine* _engine, TableCell* _parent, unsigned int _memberNumber);
        friend class TableCell;
    };

    class AbilitySlot : public ImageBox
    {
        unsigned int slotNumber{};

      public:
        void RetrieveInfo();
        void ReceiveDrop(CellElement* droppedElement) override;
        void HoverUpdate() override;
        void Draw2D() override;
        void OnClick() override;
        AbilitySlot(GameUIEngine* _engine, TableCell* _parent, unsigned int _slotNumber);
        friend class TableCell;
    };

    class ItemSlot : public ImageBox
    {
      protected:
        Texture backgroundTex{};
        void dropItemInWorld();
        virtual void onItemDroppedToWorld() = 0;
        void updateRectangle(
            const Dimensions& dimensions, const Vector2& offset, const Dimensions& space) override;

        [[nodiscard]] virtual entt::entity getItemId() = 0;
        [[nodiscard]] virtual Texture getEmptyTex();

      public:
        void Draw2D() override;
        virtual void RetrieveInfo();
        void OnDrop(CellElement* receiver) override;
        void HoverUpdate() override;
        ItemSlot(
            GameUIEngine* _engine,
            TableCell* _parent,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
        friend class TableCell;
    };

    class EquipmentSlot : public ItemSlot
    {
        [[nodiscard]] bool validateDrop(const ItemComponent& item) const;

      protected:
        Texture getEmptyTex() override;
        void onItemDroppedToWorld() override;
        [[nodiscard]] entt::entity getItemId() override;

      public:
        EquipmentSlotName itemType;
        void ReceiveDrop(CellElement* droppedElement) override;
        EquipmentSlot(GameUIEngine* _engine, TableCell* _parent, EquipmentSlotName _itemType);
        friend class TableCell;
    };

    class InventorySlot : public ItemSlot
    {

      protected:
        void onItemDroppedToWorld() override;
        [[nodiscard]] entt::entity getItemId() override;

      public:
        unsigned int row{};
        unsigned int col{};
        void ReceiveDrop(CellElement* droppedElement) override;
        InventorySlot(GameUIEngine* _engine, TableCell* _parent, unsigned int _row, unsigned int _col);
        friend class TableCell;
    };

    class CloseButton final : public ImageBox
    {
      public:
        ~CloseButton() override = default;
        void OnClick() override;
        CloseButton(GameUIEngine* _engine, TableCell* _parent, const Texture& _tex);
    };

    class TableCell final : public TableElement<std::unique_ptr<CellElement>, TableRow>
    {
        float requestedWidth{};
        bool autoSize = true;

      public:
        TextBox* CreateTextbox(std::unique_ptr<TextBox> _textBox, const std::string& _content);
        DialogOption* CreateDialogOption(std::unique_ptr<DialogOption> _dialogOption);
        TitleBar* CreateTitleBar(std::unique_ptr<TitleBar> _titleBar, const std::string& _title);
        ImageBox* CreateImagebox(std::unique_ptr<ImageBox> _imageBox);
        EquipmentCharacterPreview* CreateEquipmentCharacterPreview(
            std::unique_ptr<EquipmentCharacterPreview> _preview);
        CloseButton* CreateCloseButton(std::unique_ptr<CloseButton> _closeButton);
        PartyMemberPortrait* CreatePartyMemberPortrait(std::unique_ptr<PartyMemberPortrait> _portrait);

        AbilitySlot* CreateAbilitySlot(std::unique_ptr<AbilitySlot> _slot);
        EquipmentSlot* CreateEquipmentSlot(std::unique_ptr<EquipmentSlot> _slot);
        InventorySlot* CreateInventorySlot(std::unique_ptr<InventorySlot> _slot);
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~TableCell() override = default;
        explicit TableCell(TableRow* _parent, Padding _padding = {0, 0, 0, 0});
        friend class TableRow;
    };

    class TableRow final : public TableElement<std::vector<std::unique_ptr<TableCell>>, Table>
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableCell* CreateTableCell(Padding _padding = {0, 0, 0, 0});
        TableCell* CreateTableCell(float _requestedWidth, Padding _padding = {0, 0, 0, 0});
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~TableRow() override = default;
        explicit TableRow(Table* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Table;
    };

    class Table : public TableElement<std::vector<std::unique_ptr<TableRow>>, Panel>
    {
        float requestedWidth{};
        bool autoSize = true;

      public:
        TableRow* CreateTableRow(Padding _padding = {0, 0, 0, 0});
        TableRow* CreateTableRow(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~Table() override = default;
        explicit Table(Panel* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Panel;
    };

    class TableGrid final : public Table
    {
        float cellSpacing = 0;

      public:
        void UpdateChildren() override;
        explicit TableGrid(Panel* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Panel;
    };

    class Panel : public TableElement<std::vector<std::unique_ptr<Table>>, Window>
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableGrid* CreateTableGrid(int rows, int cols, float cellSpacing = 0);
        Table* CreateTable(Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(float _requestedWidth, Padding _padding = {0, 0, 0, 0});
        void DrawDebug2D() override;
        void Draw2D() override;
        void UpdateChildren() override;
        explicit Panel(Window* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Window;
    };

    class Window : public TableElement<std::vector<std::unique_ptr<Panel>>, void>
    {
      protected:
        bool hidden = false;
        bool markForRemoval = false;

      public:
        entt::sigh<void()> onHide;
        entt::connection windowUpdateCnx{};
        bool mouseHover = false;
        const Settings* settings{}; // for screen width/height

        void OnWindowUpdate(Vector2 prev, Vector2 current);
        virtual void ScaleContents();
        void ClampToScreen();
        void OnHoverStart() override;
        void OnHoverStop() override;
        Panel* CreatePanel(Padding _padding = {0, 0, 0, 0});
        Panel* CreatePanel(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void ToggleHide();
        void Show();
        void Hide();
        [[nodiscard]] bool IsHidden() const;
        [[nodiscard]] bool IsMarkedForRemoval() const;
        void Remove();
        void DrawDebug2D() override;
        void Draw2D() override;
        void UpdateChildren() override;
        ~Window() override;
        explicit Window(Settings* _settings, Padding _padding = {0, 0, 0, 0});
        Window(Settings* _settings, float x, float y, float width, float height, Padding _padding = {0, 0, 0, 0});
        friend class GameUIEngine;
    };

    class TooltipWindow final : public Window
    {
        entt::connection parentWindowHideCnx;

      public:
        void ScaleContents() override;
        ~TooltipWindow() override;
        TooltipWindow(
            Settings* _settings,
            Window* parentWindow,
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
        void SetAlignment(VertAlignment vert, HoriAlignment hori);
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;

      public:
        void ScaleContents() override;
        WindowDocked(
            Settings* _settings,
            VertAlignment _vertAlignment,
            HoriAlignment _horiAlignment,
            Padding _padding = {0, 0, 0, 0});
        friend class GameUIEngine;
    };

    class UIState
    {
      protected:
        CellElement* element{};
        GameUIEngine* engine{};

      public:
        virtual void Update(){};
        virtual void Draw(){};
        virtual void Enter(){};
        virtual void Exit(){};
        virtual ~UIState() = default;
        explicit UIState(CellElement* _element, GameUIEngine* _engine);
    };

    class IdleState : public UIState
    {
      public:
        void Enter() override;
        void Update() override;
        void Exit() override;
        ~IdleState() override = default;
        explicit IdleState(CellElement* _element, GameUIEngine* _engine);
    };

    class HoverState : public UIState
    {
        Timer dragTimer;

      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        ~HoverState() override = default;
        explicit HoverState(CellElement* _element, GameUIEngine* _engine);
    };

    class DragDelayState : public UIState
    {
        Timer dragTimer;

      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        ~DragDelayState() override = default;
        explicit DragDelayState(CellElement* _element, GameUIEngine* _engine);
    };

    class DragState : public UIState
    {
      public:
        void Enter() override;
        void Exit() override;
        void Update() override;
        void Draw() override;
        ~DragState() override = default;
        explicit DragState(CellElement* _element, GameUIEngine* _engine);
    };

    class GameUIEngine
    {
        std::vector<std::unique_ptr<Window>> windows;
        std::unique_ptr<TooltipWindow> tooltipWindow;
        std::optional<CellElement*> draggedObject;
        std::optional<CellElement*> hoveredDraggableCellElement;

        void pruneWindows();
        void processWindows();
        void onWorldItemHover(entt::entity entity) const;
        void onWorldCombatableHover(entt::entity entity) const;
        void onNPCHover(entt::entity entity) const;
        void onStopWorldHover() const;

        [[nodiscard]] bool mouseInNonObscuredWindowRegion(Window* window, Vector2 mousePos) const;

      public:
        entt::registry* registry;
        GameData* gameData;
        void BringClickedWindowToFront(Window* clicked);
        TooltipWindow* CreateTooltipWindow(
            Window* parentWindow,
            const Texture& _nPatchTexture,
            TextureStretchMode _textureStretchMode,
            float x,
            float y,
            float _width,
            float _height,
            Padding _padding = {0, 0, 0, 0});

        Window* CreateWindow(
            Texture _nPatchTexture,
            TextureStretchMode _textureStretchMode,
            float x,
            float y,
            float _width,
            float _height,
            Padding _padding = {0, 0, 0, 0});

        WindowDocked* CreateWindowDocked(
            float _xOffset,
            float _yOffset,
            float _width,
            float _height,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT,
            Padding _padding = {0, 0, 0, 0});

        WindowDocked* CreateWindowDocked(
            Texture _nPatchTexture,
            TextureStretchMode _textureStretchMode,
            float _xOffset,
            float _yOffset,
            float _width,
            float _height,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT,
            Padding _padding = {0, 0, 0, 0});

        [[nodiscard]] static Rectangle GetOverlap(Rectangle rec1, Rectangle rec2);
        [[nodiscard]] bool ObjectBeingDragged() const;
        void PlaceWindow(Window* window, Vector2 requestedPos) const;
        [[nodiscard]] Window* GetWindowCollision(const Window* toCheck) const;
        [[nodiscard]] CellElement* GetCellUnderCursor() const;
        void DrawDebug2D() const;
        void Draw2D() const;
        void Update();

        GameUIEngine(entt::registry* _registry, GameData* _gameData);
        friend class UIState;
        friend class DragDelayState;
        friend class DragState;
        friend class HoverState;
    };
} // namespace sage
