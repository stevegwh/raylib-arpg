//
// Created by Steve Wheeler on 03/10/2024.
//

#include "GameUiFactory.hpp"

#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "components/DialogComponent.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/Renderable.hpp"
#include "GameUiEngine.hpp"
#include "ResourceManager.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"

#include "magic_enum.hpp"
#include <format>

namespace sage
{

    Window* GameUiFactory::CreatePartyPortraitsColumn(GameUIEngine* engine)
    {
        auto portraitWidth = 132;
        auto portraitHeight = 166;
        auto w = portraitWidth;
        auto h = portraitHeight * PARTY_MEMBER_MAX;
        auto _windowDocked = std::make_unique<WindowDocked>(
            engine->sys->settings, 16, 16, w, h, VertAlignment::TOP, HoriAlignment::LEFT);
        auto* window = engine->CreateWindowDocked(std::move(_windowDocked));
        auto table = window->CreateTable();

        for (int i = 0; i < PARTY_MEMBER_MAX; ++i)
        {
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            auto portrait = std::make_unique<PartyMemberPortrait>(engine, cell, i, w, 166);
            cell->CreatePartyMemberPortrait(std::move(portrait));
        }

        window->FinalizeLayout();
        return window;
    }

    Window* GameUiFactory::CreateAbilityRow(GameUIEngine* engine)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/window_hud.png");

        auto w = 1024; // Absolute value of the image
        auto h = 156;
        auto _windowDocked = std::make_unique<WindowDocked>(
            engine->sys->settings,
            nPatchTexture,
            TextureStretchMode::STRETCH,
            0,
            0,
            w,
            h,
            VertAlignment::BOTTOM,
            HoriAlignment::CENTER,
            Padding{0, 0, 0, 0});
        auto window = engine->CreateWindowDocked(std::move(_windowDocked));

        auto tableMain = window->CreateTable();
        auto tableMainRow = tableMain->CreateTableRow();
        tableMainRow->CreateTableCell(15.5);
        auto abilityCell = tableMainRow->CreateTableCell({16, 0, 0, 0});
        tableMainRow->CreateTableCell(15.5);

        auto table = abilityCell->CreateTable();
        table->CreateTableRow(24); // Experience bar
        auto abilityRow = table->CreateTableRowGrid(MAX_ABILITY_NUMBER, 4, {4, 4, 0, 0});
        for (unsigned int i = 0; i < abilityRow->children.size(); ++i)
        {
            auto cell = dynamic_cast<TableCell*>(abilityRow->children[i].get());
            cell->CreateAbilitySlot(std::make_unique<AbilitySlot>(engine, cell, i));
        }

        // TODO: Currently, if one imagebox has SHRINK_ROW_TO_FIT all imageboxes in that row would be scaled.
        window->FinalizeLayout();
        return window;
    }

    TooltipWindow* GameUiFactory::CreateWorldTooltip(GameUIEngine* engine, const std::string& name, Vector2 pos)
    {

        // Set window's dimensions to the size of the text
        Padding padding = {10, 10, 10, 20};

        TextBox::FontInfo _fontInfo{};
        _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::SHRINK_TO_FIT;
        float scaleFactor = engine->sys->settings->GetCurrentScaleFactor();
        _fontInfo.fontSize = _fontInfo.baseFontSize * scaleFactor;
        _fontInfo.fontSize =
            std::clamp(_fontInfo.fontSize, TextBox::FontInfo::minFontSize, TextBox::FontInfo::maxFontSize);

        Vector2 textSize = MeasureTextEx(_fontInfo.font, name.c_str(), _fontInfo.fontSize, _fontInfo.fontSpacing);
        auto w = textSize.x + padding.left + padding.right;
        auto h = textSize.y + padding.up + padding.down;

        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");

        auto tooltip = std::make_unique<TooltipWindow>(
            engine->sys->settings, nullptr, nPatchTexture, TextureStretchMode::NONE, pos.x, pos.y, w, h, padding);

        auto* window = engine->CreateTooltipWindow(std::move(tooltip));

        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();

            auto textbox = std::make_unique<TextBox>(engine, cell0, _fontInfo);
            cell0->CreateTextbox(std::move(textbox), name);
        }

        window->FinalizeLayout();
        return window;
    }

    TooltipWindow* GameUiFactory::CreateItemTooltip(
        GameUIEngine* engine, ItemComponent& item, Window* parentWindow, Vector2 pos)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.1;
        auto tooltip = std::make_unique<TooltipWindow>(
            engine->sys->settings,
            parentWindow,
            nPatchTexture,
            TextureStretchMode::NONE,
            pos.x,
            pos.y,
            w,
            h,
            Padding{20, 20, 10, 6});
        auto* window = engine->CreateTooltipWindow(std::move(tooltip));

        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::WORD_WRAP;
            auto headerTextbox = std::make_unique<TextBox>(engine, cell0, _fontInfo, VertAlignment::BOTTOM);
            cell0->CreateTextbox(std::move(headerTextbox), item.localizedName);
            auto row = table->CreateTableRow({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            auto bodyTextbox = std::make_unique<TextBox>(engine, cell, _fontInfo);
            cell->CreateTextbox(std::move(bodyTextbox), item.description);
        }
        window->FinalizeLayout();
        return window;
    }

    TooltipWindow* GameUiFactory::CreateAbilityToolTip(GameUIEngine* engine, const Ability& ability, Vector2 pos)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.1;
        auto tooltip = std::make_unique<TooltipWindow>(
            engine->sys->settings,
            nullptr,
            nPatchTexture,
            TextureStretchMode::NONE,
            pos.x,
            pos.y,
            w,
            h,
            Padding{20, 20, 10, 6});
        auto* window = engine->CreateTooltipWindow(std::move(tooltip));

        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::WORD_WRAP;
            auto headerTextbox = std::make_unique<TextBox>(engine, cell0, _fontInfo, VertAlignment::BOTTOM);
            cell0->CreateTextbox(std::move(headerTextbox), ability.name);
            auto row = table->CreateTableRow({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            auto bodyTextbox = std::make_unique<TextBox>(engine, cell, _fontInfo);
            cell->CreateTextbox(std::move(bodyTextbox), ability.description);
        }
        window->FinalizeLayout();
        return window;
    }

    Window* GameUiFactory::CreateInventoryWindow(
        entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto _window = std::make_unique<Window>(
            engine->sys->settings,
            nPatchTexture,
            TextureStretchMode::STRETCH,
            pos.x,
            pos.y,
            274 * 1.5,
            424 * 1.5,
            Padding{20, 0, 14, 14});
        auto window = engine->CreateWindow(std::move(_window));
        auto mainTable = window->CreateTable();
        auto mainTableRow1 = mainTable->CreateTableRow(4);
        auto mainTableRow2 = mainTable->CreateTableRow({20, 0, 0, 0});

        {
            // Title bar
            auto cell = mainTableRow1->CreateTableCell(80);
            auto cell2 = mainTableRow1->CreateTableCell(20);
            auto titleBar = std::make_unique<TitleBar>(engine, cell, TextBox::FontInfo{});
            cell->CreateTitleBar(std::move(titleBar), "Inventory");
            auto tex = ResourceManager::GetInstance().TextureLoad("IMG_UI_CLOSE");
            auto closeBtn = std::make_unique<CloseButton>(engine, cell2, tex);
            cell2->CreateCloseButton(std::move(closeBtn));
        }

        {
            // Inventory grid
            auto cell = mainTableRow2->CreateTableCell();
            auto table = cell->CreateTableGrid(INVENTORY_MAX_ROWS, INVENTORY_MAX_COLS, 4);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto cell = dynamic_cast<TableCell*>(table->children[row]->children[col].get());
                    auto invSlot = std::make_unique<InventorySlot>(engine, cell, row, col);
                    auto ptr = cell->CreateInventorySlot(std::move(invSlot));

                    engine->sys->inventorySystem->onInventoryUpdated.Subscribe([ptr]() { ptr->RetrieveInfo(); });
                }
            }
        }
        window->FinalizeLayout();
        window->Hide();
        return window;
    }

    Window* GameUiFactory::CreateCharacterWindow(
        entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h)
    {

        // auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        // auto _window = std::make_unique<Window>(
        //     engine->sys->settings,
        //     nPatchTexture,
        //     TextureStretchMode::STRETCH,
        //     pos.x,
        //     pos.y,
        //     274 * 3,
        //     424 * 1.5,
        //     Padding{20, 20, 14, 14});
        // auto window = engine->CreateWindow(std::move(_window));
        //
        // {
        //     // TODO: Having the concept of "margins" would make it so much easier to create consistent layouts
        //     // where you don't have to request different heights and add padding to make things line up
        //     const auto panel1 = window->CreatePanel(10);
        //     const auto table = panel1->CreateTable();
        //     const auto row = table->CreateTableRow();
        //     const auto cell = row->CreateTableCell(80);
        //     const auto cell2 = row->CreateTableCell(20, {0, 18 * 2, 18, 18});
        //     auto titleText = std::make_unique<TitleBar>(engine, cell, TextBox::FontInfo{});
        //     cell->CreateTitleBar(std::move(titleText), "Character");
        //
        //     const auto tex = ResourceManager::GetInstance().TextureLoad("IMG_UI_CLOSE");
        //     auto closeBtn = std::make_unique<CloseButton>(engine, cell2, tex);
        //     cell2->CreateCloseButton(std::move(closeBtn));
        // }
        //
        // int maxRows = 6;
        // int maxCols = 1;
        //
        // auto createEquipSlot =
        //     [&engine](const Table* table, unsigned int row, unsigned int col, EquipmentSlotName itemType) {
        //         auto cell = dynamic_cast<TableCell*>(table->children[row]->children[col].get());
        //         auto equipSlot = std::make_unique<EquipmentSlot>(engine, cell, itemType);
        //         auto* ptr = cell->CreateEquipmentSlot(std::move(equipSlot));
        //         engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe(
        //             [ptr](entt::entity) { ptr->RetrieveInfo(); });
        //     };
        //
        // auto createSpacerSlot = [&engine](const Table* table, unsigned int row, unsigned int col) {
        //     auto cell = dynamic_cast<TableCell*>(table->children[row]->children[col].get());
        //     auto imgBox = std::make_unique<ImageBox>(
        //         engine, cell, ResourceManager::GetInstance().TextureLoad("resources/transpixel.png"));
        //     cell->CreateImagebox(std::move(imgBox));
        // };
        //
        // auto panel2 = window->CreatePanel({0, 0, 4, 0});
        // panel2->SetTexture(
        //     ResourceManager::GetInstance().TextureLoad("resources/textures/ui/inventory-bg.png"),
        //     TextureStretchMode::STRETCH);
        // {
        //     auto table = panel2->CreateTableGrid(maxRows, maxCols, 4);
        //     for (unsigned int row = 0; row < maxRows; ++row)
        //     {
        //         for (unsigned int col = 0; col < maxCols; ++col)
        //         {
        //             createSpacerSlot(table, row, col);
        //         }
        //     }
        //
        //     createEquipSlot(table, 0, 0, EquipmentSlotName::HELM);
        //     createEquipSlot(table, 1, 0, EquipmentSlotName::ARMS);
        //     createEquipSlot(table, 2, 0, EquipmentSlotName::CHEST);
        //     createEquipSlot(table, 3, 0, EquipmentSlotName::AMULET);
        //     createEquipSlot(table, 4, 0, EquipmentSlotName::LEGS);
        //     createEquipSlot(table, 5, 0, EquipmentSlotName::LEFTHAND);
        // }
        //
        // {
        //     // Character model
        //     auto table = panel2->CreateTable(40, {0, 0, 24, 24});
        //     auto row = table->CreateTableRow();
        //     auto cell = row->CreateTableCell();
        //     auto preview = std::make_unique<EquipmentCharacterPreview>(engine, cell);
        //     auto img = cell->CreateEquipmentCharacterPreview(std::move(preview));
        //     img->draggable = false;
        // }
        //
        // {
        //     auto table = panel2->CreateTableGrid(maxRows, maxCols, 4);
        //
        //     for (unsigned int row = 0; row < maxRows; ++row)
        //     {
        //         for (unsigned int col = 0; col < maxCols; ++col)
        //         {
        //             createSpacerSlot(table, row, col);
        //         }
        //     }
        //
        //     createEquipSlot(table, 0, 0, EquipmentSlotName::BELT);
        //     createEquipSlot(table, 1, 0, EquipmentSlotName::BOOTS);
        //     createEquipSlot(table, 2, 0, EquipmentSlotName::RING2);
        //     createEquipSlot(table, 3, 0, EquipmentSlotName::RING1);
        //     createEquipSlot(table, 5, 0, EquipmentSlotName::RIGHTHAND);
        // }
        //
        // {
        //     // Character statistics
        //     auto table = panel2->CreateTable(40, {50, 250, 48, 48});
        //     // ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/window_dialogue.png");
        //     table->SetTexture(
        //         ResourceManager::GetInstance().TextureLoad("resources/textures/ui/scroll-bg.png"),
        //         TextureStretchMode::STRETCH);
        //
        //     for (int i = 0; i < magic_enum::enum_underlying(CharacterStatText::StatisticType::COUNT); ++i)
        //     {
        //         auto row = table->CreateTableRow();
        //         auto cell = row->CreateTableCell();
        //         auto id = magic_enum::enum_cast<CharacterStatText::StatisticType>(i).value();
        //         auto stat = std::make_unique<CharacterStatText>(engine, cell, TextBox::FontInfo{}, id);
        //         cell->CreateCharacterStatText(std::move(stat));
        //     }
        // }
        //
        // window->FinalizeLayout();
        // window->Hide();
        // return window;
    }

    Window* GameUiFactory::CreateDialogWindow(GameUIEngine* engine, entt::entity npc)
    {
        // const auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");
        //
        // float w = Settings::TARGET_SCREEN_WIDTH * 0.65;
        // float h = Settings::TARGET_SCREEN_HEIGHT * 0.35;
        // auto _windowDocked = std::make_unique<WindowDocked>(
        //     engine->sys->settings,
        //     nPatchTexture,
        //     TextureStretchMode::NONE,
        //     0,
        //     0,
        //     w,
        //     h,
        //     VertAlignment::BOTTOM,
        //     HoriAlignment::CENTER,
        //     Padding{32, 32, 16, 16});
        // auto window = engine->CreateWindowDocked(std::move(_windowDocked));
        // window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        //
        // auto& dialogComponent = engine->registry->get<DialogComponent>(npc);
        // TextBox::FontInfo _fontInfo;
        // _fontInfo.baseFontSize = 20;
        // _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::WORD_WRAP;
        //
        // // NPC Part
        // {
        //     auto& renderable = engine->registry->get<Renderable>(npc);
        //     auto panel = window->CreatePanel();
        //     const auto table = panel->CreateTable();
        //     const auto descriptionRow = table->CreateTableRow();
        //     const auto descriptionCell = descriptionRow->CreateTableCell({10, 10, 20, 20});
        //     auto textbox = std::make_unique<TextBox>(engine, descriptionCell, _fontInfo);
        //
        //     std::string speakerName;
        //     if (!dialogComponent.conversation->speaker.empty())
        //     {
        //         speakerName = dialogComponent.conversation->speaker;
        //     }
        //     else
        //     {
        //         speakerName = renderable.GetVanityName();
        //     }
        //
        //     descriptionCell->CreateTextbox(
        //         std::move(textbox),
        //         std::format("[{}]: {}", speakerName, dialogComponent.conversation->GetCurrentNode()->content));
        // }
        // // ----------------
        //
        // // Player part
        // {
        //     auto panel = window->CreatePanel();
        //     const auto portraitTable = panel->CreateTable();
        //     auto portraitRow = portraitTable->CreateTableRow();
        //     auto portraitCell = portraitRow->CreateTableCell();
        //
        //     const auto& info = engine->registry->get<PartyMemberComponent>(
        //         engine->sys->controllableActorSystem->GetSelectedActor());
        //     auto tex = info.portraitImg.texture;
        //     auto img = std::make_unique<DialogPortrait>(engine, portraitCell, tex);
        //     portraitCell->CreateImagebox(std::move(img));
        //
        //     const auto optionsTable = panel->CreateTable(80);
        //     unsigned int i = 0;
        //     for (const auto& o : dialogComponent.conversation->GetCurrentNode()->options)
        //     {
        //         if (!o->ShouldShow()) continue;
        //         const auto optionRow = optionsTable->CreateTableRow();
        //         const auto optionCell = optionRow->CreateTableCell({10, 10, 10, 10});
        //         auto option = std::make_unique<DialogOption>(engine, optionCell, o.get(), ++i, _fontInfo);
        //         optionCell->CreateDialogOption(std::move(option));
        //     }
        // }
        // window->FinalizeLayout();
        // return window;
    }

    Window* GameUiFactory::CreateGameWindowButtons(
        GameUIEngine* engine, Window* inventoryWindow, Window* equipmentWindow)
    {
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.075;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.075;
        auto _windowDocked = std::make_unique<WindowDocked>(
            engine->sys->settings,
            0,
            0,
            w,
            h,
            VertAlignment::BOTTOM,
            HoriAlignment::LEFT,
            Padding{16, 16, 12, 12});

        auto window = engine->CreateWindowDocked(std::move(_windowDocked));
        window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        auto table = window->CreateTable();
        auto row = table->CreateTableRow();
        auto cell = row->CreateTableCell();
        cell->CreateGameWindowButton(std::make_unique<GameWindowButton>(
            engine,
            cell,
            ResourceManager::GetInstance().TextureLoad("resources/icons/ui/inventory.png"),
            inventoryWindow));
        auto cell1 = row->CreateTableCell();
        cell1->CreateGameWindowButton(std::make_unique<GameWindowButton>(
            engine,
            cell1,
            ResourceManager::GetInstance().TextureLoad("resources/icons/ui/equipment.png"),
            equipmentWindow));
        window->FinalizeLayout();
        return window;
    }
} // namespace sage