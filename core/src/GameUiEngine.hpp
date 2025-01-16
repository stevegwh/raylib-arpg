//
// Created by steve on 02/10/2024.
//

#pragma once

#include "Settings.hpp"

#include "components/DialogComponent.hpp"
#include "Event.hpp"
#include "ResourceManager.hpp"
#include "Timer.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

// #include <memory>
#include <optional>
// #include <utility>
#include <vector>

namespace sage
{
    class TableGrid;
    class TooltipWindow;
    class Systems;
    class GameUIEngine;
    struct Settings;
    class UserInput;
    class Cursor;
    struct ItemComponent;
    class EquipmentComponent;
    enum class EquipmentSlotName;
    class PartySystem;
    class Window;
    class Table;
    class TableRow;
    class TableCell;
    class UIState;
    class PlayerAbilitySystem;
    class ControllableActorSystem;
    class QuestManager;

    enum class TextureStretchMode
    {
        NONE,
        STRETCH, // maximises or minimises image to fit, ignores aspect ratio, whole image is kept
        FILL,    // maximises or minimises image to fit, maintains aspect ratio (parts that fall outside of aspect
                 // ratio are clipped)
        TILE     // Repeats image if not big enough to fill rec
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

    class UIElement
    {
      protected:
      public:
        Rectangle rec{};
        [[nodiscard]] const Rectangle& GetRec() const
        {
            return rec;
        }
        virtual void OnIdleStart(){};
        virtual void OnIdleStop(){};
        virtual void OnHoverStart();
        virtual void OnHoverStop();
        virtual ~UIElement() = default;
        UIElement() = default;
    };

    class CellElement : public UIElement
    {
      protected:
        Event<> onMouseClicked;
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

        virtual void RetrieveInfo(){};
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
            SHRINK_TO_FIT, // Text will be shrunk down to a minimum size (then left to overflow)
            WORD_WRAP // Words will be broken onto a new line if they do not fit. Will be shrunk if the new line
                      // also does not fit. Does not truncate words.
            // SHRINK_THEN_EXTEND ?
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
            const FontInfo& _fontInfo = FontInfo(),
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);

      protected:
        Shader sdfShader;
        FontInfo fontInfo;
        std::string content;
    };

    class TableElement : public UIElement
    {
      protected:
        struct UnscaledDimensions
        {
            Rectangle rec{};
            Padding padding{};
            Vector2 scalePosOffset{}; // The offset between the old and new screen size
        };

        TextureStretchMode textureStretchMode = TextureStretchMode::NONE;
        virtual void Reset();

      public:
        Padding padding;
        UnscaledDimensions unscaledDimensions{};
        TableElement* parent{};
        std::vector<std::unique_ptr<TableElement>> children;
        std::optional<std::unique_ptr<CellElement>> element;
        std::optional<Texture> tex{};
        std::optional<NPatchInfo> nPatchInfo{};

        virtual CellElement* GetCellUnderCursor();
        void OnHoverStop() override;
        virtual void Update();
        virtual void ScaleContents(Settings* _settings);
        virtual void SetPos(float x, float y);
        void SetDimensions(float w, float h);
        void SetTexture(const Texture& _tex, TextureStretchMode _stretchMode);
        void UpdateTextureDimensions();
        virtual void FinalizeLayout();
        virtual void InitLayout() = 0;
        virtual void DrawDebug2D();
        virtual void Draw2D();
        [[nodiscard]] Window* GetWindow();

        TableElement(TableElement* _parent, float x, float y, float width, float height, Padding _padding);
        TableElement(TableElement* _parent, Padding _padding);

        TableElement(const TableElement&) = default;
        TableElement(TableElement&&) noexcept = default;
        TableElement& operator=(const TableElement&) = default;
        TableElement& operator=(TableElement&&) noexcept = default;
        ~TableElement() override = default;
    };

    // Also displays description
    class JournalEntryManager : public TextBox
    {
        TableCell* journalEntryRoot;
        QuestManager* questManager;

        void updateQuests();

      public:
        JournalEntryManager(
            GameUIEngine* _engine,
            TableCell* _parent,
            TableCell* _journalEntryRoot,
            QuestManager* _questManager,
            const FontInfo& _fontInfo,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
    };

    class JournalEntry : public TextBox
    {
        entt::entity questId;
        Quest* quest;
        TableCell* descriptionCell;
        bool drawHighlight = false;

      public:
        void OnHoverStart() override;
        void OnHoverStop() override;
        void Draw2D() override;
        void OnClick() override;
        JournalEntry(
            GameUIEngine* _engine,
            TableCell* _parent,
            TableCell* _descriptionCell,
            Quest* _quest,
            const FontInfo& _fontInfo,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
    };

    class DialogOption : public TextBox
    {
        dialog::Option* option;
        unsigned int index{};
        bool drawHighlight = false;

      public:
        void OnHoverStart() override;
        void OnHoverStop() override;
        void Draw2D() override;
        void OnClick() override;
        DialogOption(
            GameUIEngine* _engine,
            TableCell* _parent,
            dialog::Option* _option,
            unsigned int _index,
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

    class CharacterStatText final : public TextBox
    {

      public:
        enum class StatisticType
        {
            NAME,
            STRENGTH,
            AGILITY,
            INTELLIGENCE,
            CONSTITUTION,
            WITS,
            MEMORY,
            COUNT // must be last
        };
        StatisticType statisticType;
        void RetrieveInfo() override;
        ~CharacterStatText() override = default;
        CharacterStatText(
            GameUIEngine* _engine, TableCell* _parent, const FontInfo& _fontInfo, StatisticType _statisticType);
    };

    class ImageBox : public CellElement
    {
      public:
        enum class OverflowBehaviour
        {
            ALLOW_OVERFLOW,
            SHRINK_TO_FIT,
            SHRINK_ROW_TO_FIT,
            SHRINK_COL_TO_FIT
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
        OverflowBehaviour overflowBehaviour;
        std::optional<Shader> shader;
        virtual void updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space);

      private:
        [[nodiscard]] Dimensions calculateAvailableSpace() const;
        [[nodiscard]] float calculateAspectRatio() const;
        [[nodiscard]] Dimensions calculateInitialDimensions(const Dimensions& space) const;
        [[nodiscard]] Vector2 calculateAlignmentOffset(
            const Dimensions& dimensions, const Dimensions& space) const;
        void shrinkRowToFit() const;
        void shrinkColToFit() const;
        [[nodiscard]] size_t findMyColumnIndex() const;
        [[nodiscard]] Dimensions handleOverflow(const Dimensions& dimensions, const Dimensions& space) const;
    };

    class GameWindowButton : public ImageBox
    {
        Window* toOpen{};

      public:
        void OnClick() override;
        GameWindowButton(GameUIEngine* _engine, TableCell* _parent, const Texture& _tex, Window* _toOpen);
    };

    class ResourceOrb : public ImageBox // Health, mana, etc.
    {

      public:
        void RetrieveInfo() override;
        void Draw2D() override;
        ResourceOrb(
            GameUIEngine* _engine,
            TableCell* _parent,
            VertAlignment _vertAlignment = VertAlignment::TOP,
            HoriAlignment _horiAlignment = HoriAlignment::LEFT);
    };

    class EquipmentCharacterPreview : public ImageBox
    {

      public:
        void UpdateDimensions() override;
        void RetrieveInfo() override;
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
        int width;
        int height;

      public:
        void HoverUpdate() override;
        void UpdateDimensions() override;
        void RetrieveInfo() override;
        void ReceiveDrop(CellElement* droppedElement) override;
        void OnClick() override;
        void Draw2D() override;
        PartyMemberPortrait(
            GameUIEngine* _engine, TableCell* _parent, unsigned int _memberNumber, int _width, int _height);
        friend class TableCell;
    };

    class DialogPortrait : public ImageBox
    {

      public:
        void Draw2D() override;
        DialogPortrait(GameUIEngine* _engine, TableCell* _parent, const Texture& _tex);
        friend class TableCell;
    };

    class AbilitySlot : public ImageBox
    {
        unsigned int slotNumber{};

      public:
        void RetrieveInfo() override;
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
        void RetrieveInfo() override;
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
        entt::entity owner{};
        void onItemDroppedToWorld() override;
        [[nodiscard]] entt::entity getItemId() override;

      public:
        unsigned int row{};
        unsigned int col{};
        entt::entity GetOwner() const;
        void SetOwner(entt::entity _owner);
        void ReceiveDrop(CellElement* droppedElement) override;
        InventorySlot(
            GameUIEngine* _engine, TableCell* _parent, entt::entity _owner, unsigned int _row, unsigned int _col);
        friend class TableCell;
    };

    class CloseButton final : public ImageBox
    {
        bool closeDeletesWindow = false;

      public:
        ~CloseButton() override = default;
        void OnClick() override;
        CloseButton(
            GameUIEngine* _engine, TableCell* _parent, const Texture& _tex, bool _closeDeletesWindow = false);
    };

    class TableCell final : public TableElement
    {
        float requestedWidth{};
        bool autoSize = true;

      public:
        // TODO: use polymorphism for any duplicates
        TextBox* CreateTextbox(std::unique_ptr<TextBox> _textBox, const std::string& _content);
        DialogOption* CreateDialogOption(std::unique_ptr<DialogOption> _dialogOption);
        TitleBar* CreateTitleBar(std::unique_ptr<TitleBar> _titleBar, const std::string& _title);
        CharacterStatText* CreateCharacterStatText(std::unique_ptr<CharacterStatText> _statText);
        ImageBox* CreateImagebox(std::unique_ptr<ImageBox> _imageBox);
        EquipmentCharacterPreview* CreateEquipmentCharacterPreview(
            std::unique_ptr<EquipmentCharacterPreview> _preview);
        CloseButton* CreateCloseButton(std::unique_ptr<CloseButton> _closeButton);
        PartyMemberPortrait* CreatePartyMemberPortrait(std::unique_ptr<PartyMemberPortrait> _portrait);
        GameWindowButton* CreateGameWindowButton(std::unique_ptr<GameWindowButton> _button);
        AbilitySlot* CreateAbilitySlot(std::unique_ptr<AbilitySlot> _slot);
        EquipmentSlot* CreateEquipmentSlot(std::unique_ptr<EquipmentSlot> _slot);
        InventorySlot* CreateInventorySlot(std::unique_ptr<InventorySlot> _slot);
        ResourceOrb* CreateResourceOrb(std::unique_ptr<ResourceOrb> _orb);
        TableGrid* CreateTableGrid(int rows, int cols, float cellSpacing = 0, Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void InitLayout() override;
        ~TableCell() override = default;
        explicit TableCell(TableRow* _parent, Padding _padding = {0, 0, 0, 0});
        friend class TableRow;
    };

    class TableRow : public TableElement
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableCell* CreateTableCell(Padding _padding = {0, 0, 0, 0});
        TableCell* CreateTableCell(float _requestedWidth, Padding _padding = {0, 0, 0, 0});
        void InitLayout() override;
        ~TableRow() override = default;
        explicit TableRow(Table* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Table;
    };

    class TableRowGrid final : public TableRow
    {
        float cellSpacing = 0;

      public:
        void InitLayout() override;
        explicit TableRowGrid(Table* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Table;
    };

    class Table : public TableElement
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableRowGrid* CreateTableRowGrid(int cols, float cellSpacing, Padding _padding);
        TableRow* CreateTableRow(Padding _padding = {0, 0, 0, 0});
        TableRow* CreateTableRow(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void InitLayout() override;
        ~Table() override = default;
        explicit Table(Window* _parent, Padding _padding = {0, 0, 0, 0});
        explicit Table(TableCell* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Window;
        friend class TableCell;
    };

    class TableGrid final : public Table
    {
        float cellSpacing = 0;

      public:
        void InitLayout() override;
        explicit TableGrid(Window* _parent, Padding _padding = {0, 0, 0, 0});
        explicit TableGrid(TableCell* _parent, Padding _padding = {0, 0, 0, 0});
        friend class Window;
        friend class TableCell;
    };

    class Window : public TableElement
    {
        void ScaleContents(Settings* _settings) override;

      protected:
        bool hidden = false;
        bool markForRemoval = false;

      public:
        Event<> onHide;
        Event<> onShow;
        std::unique_ptr<Connection> windowUpdateCnx{};
        bool mouseHover = false;
        Settings* settings{}; // for screen width/height

        void SetPos(float x, float y) override;
        void FinalizeLayout() override;
        void OnWindowUpdate(Vector2 prev, Vector2 current);
        void ClampToScreen();
        void OnHoverStart() override;
        TableGrid* CreateTableGrid(int rows, int cols, float cellSpacing = 0, Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(Padding _padding = {0, 0, 0, 0});
        Table* CreateTable(float _requestedHeight, Padding _padding = {0, 0, 0, 0});
        void ToggleHide();
        void Show();
        void Hide();
        [[nodiscard]] bool IsHidden() const;
        [[nodiscard]] bool IsMarkedForRemoval() const;
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
        std::unique_ptr<Connection> parentWindowHideCnx;

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
        std::optional<ErrorMessage> errorMessage;
        std::vector<std::unique_ptr<Window>> windows;
        std::unique_ptr<TooltipWindow> tooltipWindow;
        std::optional<CellElement*> draggedObject;
        std::optional<CellElement*> hoveredDraggableCellElement;

        void pruneWindows();
        void processWindows();
        void onWorldItemHover(entt::entity entity) const;
        void onNPCHover(entt::entity entity) const;
        void onStopWorldHover() const;

        [[nodiscard]] bool mouseInNonObscuredWindowRegion(Window* window, Vector2 mousePos) const;

      public:
        entt::registry* registry;
        Systems* sys;
        void BringClickedWindowToFront(Window* clicked);
        void CreateErrorMessage(const std::string& msg);
        TooltipWindow* CreateTooltipWindow(std::unique_ptr<TooltipWindow> _tooltipWindow);
        Window* CreateWindow(std::unique_ptr<Window> _window);
        WindowDocked* CreateWindowDocked(std::unique_ptr<WindowDocked> _windowDocked);

        [[nodiscard]] static Rectangle GetOverlap(Rectangle rec1, Rectangle rec2);
        [[nodiscard]] bool ObjectBeingDragged() const;
        [[nodiscard]] Window* GetWindowCollision(const Window* toCheck) const;
        [[nodiscard]] CellElement* GetCellUnderCursor() const;
        void DrawDebug2D() const;
        void Draw2D() const;
        void Update();

        GameUIEngine(entt::registry* _registry, Systems* _sys);
        friend class UIState;
        friend class DragDelayState;
        friend class DragState;
        friend class HoverState;
    };
} // namespace sage
