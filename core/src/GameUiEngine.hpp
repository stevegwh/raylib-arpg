//
// Created by steve on 02/10/2024.
//

#pragma once

#include "Settings.hpp"

#include "components/DialogComponent.hpp"
#include "raylib.h"
#include "Timer.hpp"

#include <entt/entt.hpp>
#include <optional>
#include <vector>

namespace sage
{
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

    struct Dimensions
    {
        float width;
        float height;
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
        Padding padding;
        Margin margin;

      public:
        Parent* parent{};
        Child children;

        std::optional<Texture> tex{};
        std::optional<NPatchInfo> nPatchInfo{};

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
                    DrawTextureRec(tex.value(), rec, {rec.x, rec.y}, WHITE);
                }
            }
        }

        // Sets padding by pixel value (screen scaling is applied)
        void SetPadding(const Padding& _padding)
        {
            padding = _padding;
            auto* window = GetWindow();
            const float scaleFactor = window->settings->GetScreenScaleFactor();
            padding.up *= scaleFactor;
            padding.down *= scaleFactor;
            padding.left *= scaleFactor;
            padding.right *= scaleFactor;
            UpdateChildren();
        }

        // Set padding in percent of screen
        void SetPaddingPercent(const Padding& _padding)
        {
            auto* window = GetWindow();
            float screenWidth = window->settings->screenWidth;
            float screenHeight = window->settings->screenHeight;
            padding.up = screenHeight * (_padding.up / 100);
            padding.down = screenHeight * (_padding.down / 100);
            padding.left = screenWidth * (_padding.left / 100);
            padding.right = screenWidth * (_padding.right / 100);
            UpdateChildren();
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

        TableElement() = default;
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
        GameUIEngine* engine{};
        TableCell* parent{};
        std::unique_ptr<UIState> state;
        Texture tex{};
        bool canReceiveDragDrops = false;
        bool draggable = false;
        bool beingDragged = false;
        bool stateLocked = false;
        float dragDelayTime = 0.1;

        void SetVertAlignment(VertAlignment alignment);
        void SetHoriAlignment(HoriAlignment alignment);
        virtual void OnClick();
        virtual void HoverUpdate();
        virtual void OnDragStart();
        virtual void DragUpdate(){};
        virtual void DragDraw(){};
        virtual void OnDrop(CellElement* receiver);
        virtual void ReceiveDrop(CellElement* droppedElement);
        void ChangeState(std::unique_ptr<UIState> newState);
        virtual void UpdateDimensions() = 0;
        virtual void Draw2D() = 0;
        explicit CellElement(GameUIEngine* _engine);
        ~CellElement() override = default;
        friend class TableCell;
    };

    class TextBox : public CellElement
    {
      protected:
        Shader sdfShader;
        float baseFontSize = 14;
        float fontSize = 14;
        float fontSpacing = 2;
        Font font = GetFontDefault();
        const float minFontSize = 14.0f;
        const float maxFontSize = 72.0f;

      public:
        enum class OverflowBehaviour
        {
            SHRINK_TO_FIT,
            WORD_WRAP
        };
        OverflowBehaviour overflowBehaviour = OverflowBehaviour::SHRINK_TO_FIT;
        // color?
        std::string content;
        [[nodiscard]] Font GetFont() const;
        void SetFont(const Font& _font, float _baseFontSize);
        void SetOverflowBehaviour(OverflowBehaviour _behaviour);
        void UpdateFontScaling();
        void UpdateDimensions() override;
        void Draw2D() override;
        ~TextBox() override = default;
        explicit TextBox(GameUIEngine* _engine);
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
        DialogOption(GameUIEngine* _engine, const dialog::Option& _option);
    };

    class TitleBar final : public TextBox
    {

      public:
        std::optional<Window*> draggedWindow;
        ~TitleBar() override = default;
        void OnDragStart() override;
        void DragUpdate() override;
        void OnDrop(CellElement* droppedElement) override;
        explicit TitleBar(GameUIEngine* _engine);
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
        void SetOverflowBehaviour(OverflowBehaviour _behaviour);
        void SetHoverShader();
        void SetGrayscale();
        void RemoveShader();
        void UpdateDimensions() override;
        void Draw2D() override;
        explicit ImageBox(GameUIEngine* _engine);
        ~ImageBox() override = default;

      protected:
        double hoverTimer = 0;
        float hoverTimerThreshold = 0.8;
        std::optional<Window*> tooltipWindow;
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
        explicit EquipmentCharacterPreview(GameUIEngine* _engine);
    };

    class PartyMemberPortrait : public ImageBox
    {
        PartySystem* partySystem{};
        ControllableActorSystem* controllableActorSystem{};
        unsigned int memberNumber{};

      public:
        void RetrieveInfo();
        void ReceiveDrop(CellElement* droppedElement) override;
        void OnClick() override;
        void Draw2D() override;
        explicit PartyMemberPortrait(GameUIEngine* _engine);
        friend class TableCell;
    };

    class AbilitySlot : public ImageBox
    {
        PlayerAbilitySystem* playerAbilitySystem{};
        ControllableActorSystem* controllableActorSystem{};
        unsigned int slotNumber{};

      public:
        void RetrieveInfo();
        void ReceiveDrop(CellElement* droppedElement) override;
        void HoverUpdate() override;
        void Draw2D() override;
        void OnClick() override;
        explicit AbilitySlot(GameUIEngine* _engine);
        friend class TableCell;
    };

    class ItemSlot : public ImageBox
    {
      protected:
        Texture emptyTex{};
        void dropItemInWorld();
        virtual void onItemDroppedToWorld() = 0;
        void updateRectangle(
            const Dimensions& dimensions, const Vector2& offset, const Dimensions& space) override;

        [[nodiscard]] virtual entt::entity getItemId() = 0;

      public:
        void Draw2D() override;
        virtual void RetrieveInfo();
        void OnDrop(CellElement* receiver) override;
        void HoverUpdate() override;
        explicit ItemSlot(GameUIEngine* _engine);
        friend class TableCell;
    };

    class EquipmentSlot : public ItemSlot
    {
        ControllableActorSystem* controllableActorSystem{};
        [[nodiscard]] bool validateDrop(const ItemComponent& item) const;

      protected:
        void onItemDroppedToWorld() override;
        [[nodiscard]] entt::entity getItemId() override;

      public:
        EquipmentSlotName itemType;
        void ReceiveDrop(CellElement* droppedElement) override;
        explicit EquipmentSlot(GameUIEngine* _engine, EquipmentSlotName _itemType);
        friend class TableCell;
    };

    class InventorySlot : public ItemSlot
    {
        ControllableActorSystem* controllableActorSystem{};

      protected:
        void onItemDroppedToWorld() override;
        [[nodiscard]] entt::entity getItemId() override;

      public:
        unsigned int row{};
        unsigned int col{};
        void ReceiveDrop(CellElement* droppedElement) override;
        explicit InventorySlot(GameUIEngine* _engine);
        friend class TableCell;
    };

    class CloseButton final : public ImageBox
    {
      public:
        ~CloseButton() override = default;
        void OnClick() override;
        explicit CloseButton(GameUIEngine* _engine);
    };

    class TableCell final : public TableElement<std::unique_ptr<CellElement>, TableRow>
    {
        float requestedWidth{};
        bool autoSize = true;

      public:
        TextBox* CreateTextbox(
            GameUIEngine* engine,
            const std::string& _content,
            float fontSize = 16,
            TextBox::OverflowBehaviour overflowBehaviour = TextBox::OverflowBehaviour::SHRINK_TO_FIT);
        DialogOption* CreateDialogOption(
            GameUIEngine* engine,
            const dialog::Option&,
            float fontSize = 16,
            TextBox::OverflowBehaviour overflowBehaviour = TextBox::OverflowBehaviour::SHRINK_TO_FIT);
        TitleBar* CreateTitleBar(GameUIEngine* engine, const std::string& _title, float fontSize);
        ImageBox* CreateImagebox(GameUIEngine* engine, const Texture& _tex);
        EquipmentCharacterPreview* CreateEquipmentCharacterPreview(GameUIEngine* engine);
        CloseButton* CreateCloseButton(GameUIEngine* engine, const Texture& _tex);
        PartyMemberPortrait* CreatePartyMemberPortrait(
            GameUIEngine* engine,
            PartySystem* _partySystem,
            ControllableActorSystem* _controllableActorSystem,
            unsigned int _memberNumber);
        AbilitySlot* CreateAbilitySlot(
            GameUIEngine* engine,
            PlayerAbilitySystem* _playerAbilitySystem,
            ControllableActorSystem* _controllableActorSystem,
            unsigned int _slotNumber);
        EquipmentSlot* CreateEquipmentSlot(
            GameUIEngine* engine, ControllableActorSystem* _controllableActorSystem, EquipmentSlotName _itemType);
        InventorySlot* CreateInventorySlot(
            GameUIEngine* engine,
            ControllableActorSystem* _controllableActorSystem,
            unsigned int row,
            unsigned int col);
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~TableCell() override = default;
        TableCell() = default;
        friend class TableRow;
    };

    class TableRow final : public TableElement<std::vector<std::unique_ptr<TableCell>>, Table>
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableCell* CreateTableCell();
        TableCell* CreateTableCell(float _requestedWidth);
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~TableRow() override = default;
        TableRow() = default;
        friend class Table;
    };

    class Table : public TableElement<std::vector<std::unique_ptr<TableRow>>, Panel>
    {
        float requestedWidth{};
        bool autoSize = true;

      public:
        TableRow* CreateTableRow();
        TableRow* CreateTableRow(float _requestedHeight);
        void UpdateChildren() override;
        void DrawDebug2D() override;
        void Draw2D() override;
        ~Table() override = default;
        Table() = default;
        friend class Panel;
    };

    class TableGrid final : public Table
    {
        float cellSpacing = 0;

      public:
        void UpdateChildren() override;
        TableGrid() = default;
        friend class Panel;
    };

    class Panel : public TableElement<std::vector<std::unique_ptr<Table>>, Window>
    {
        float requestedHeight{};
        bool autoSize = true;

      public:
        TableGrid* CreateTableGrid(int rows, int cols, float cellSpacing = 0);
        Table* CreateTable();
        Table* CreateTable(float _requestedWidth);
        void DrawDebug2D() override;
        void Draw2D() override;
        void UpdateChildren() override;
        friend class Window;
    };

    class Window : public TableElement<std::vector<std::unique_ptr<Panel>>, void>
    {
      protected:
        bool hidden = false;
        bool markForRemoval = false;
        float baseWidth = 0;  // Width before screen scaling (Screen width: 1920)
        float baseHeight = 0; // Height before screen scaling (Screen height: 1080)

      public:
        entt::connection windowUpdateCnx{};
        bool mouseHover = false;
        const Settings* settings{}; // for screen width/height

        virtual void ScaleContents();
        void ClampToScreen();
        void OnHoverStart() override;
        void OnHoverStop() override;
        Panel* CreatePanel();
        Panel* CreatePanel(float _requestedHeight);
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
        explicit Window(Settings* _settings) : settings(_settings)
        {
        }
        friend class GameUIEngine;
    };

    class WindowDocked final : public Window
    {
        float baseXOffset = 0;
        float baseYOffset = 0;

      public:
        VertAlignment vertAlignment = VertAlignment::TOP;
        HoriAlignment horiAlignment = HoriAlignment::LEFT;

        void SetAlignment(VertAlignment vert, HoriAlignment hori);
        void ScaleContents() override;
        explicit WindowDocked(Settings* _settings);
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
        std::unique_ptr<Window> tooltipWindow;
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
        Window* CreateTooltipWindow(const Texture& _nPatchTexture, float x, float y, float _width, float _height);
        Window* CreateWindow(
            Texture _nPatchTexture, float x, float y, float _width, float _height, bool tooltip = false);

        WindowDocked* CreateWindowDocked(float _xOffset, float _yOffset, float _width, float _height);

        WindowDocked* CreateWindowDocked(
            Texture _nPatchTexture, float _xOffset, float _yOffset, float _width, float _height);

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
