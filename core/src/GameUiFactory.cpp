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
#include "GameData.hpp"
#include "GameUiEngine.hpp"
#include "ResourceManager.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"

#include <format>

namespace sage
{

    void GameUiFactory::CreateExampleWindow(GameUIEngine* engine)
    {
        // ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        // ResourceManager::GetInstance().ImageLoadFromFile("resources/icon.png");
        // ResourceManager::GetInstance().ImageLoadFromFile("resources/test.png");
        //
        // auto nPatchTexture =
        // ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        // //        info1 = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        // //        info2 = {Rectangle{0.0f, 128.0f, 64.0f, 64.0f}, 16, 16, 16, 16, NPATCH_NINE_PATCH};
        // //        info3 = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        //
        // auto w = Settings::TARGET_SCREEN_WIDTH * 0.4;
        // auto h = Settings::TARGET_SCREEN_HEIGHT * 0.2;
        // auto window = engine->CreateWindowDocked(
        //     nPatchTexture,
        //     TextureStretchMode::SCALE,
        //     0,
        //     0,
        //     w,
        //     h,
        //     VertAlignment::BOTTOM,
        //     HoriAlignment::CENTER,
        //     {2, 2, 2, 2});
        // window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        // auto panel = window->CreatePanel();
        // auto table = panel->CreateTable();
        //
        // auto row0 = table->CreateTableRow();
        // auto cell0 = row0->CreateTableCell(95);
        // cell0->CreateTitleBar(engine, "Title Bar", 24);
        // auto cell01 = row0->CreateTableCell();
        // auto tex0 = ResourceManager::GetInstance().TextureLoad("resources/icon.png");
        // auto closeBtn = std::make_unique<CloseButton>(engine, cell01, tex0);
        // cell01->CreateCloseButton(std::move(closeBtn));
        //
        // auto row = table->CreateTableRow(75);
        // auto cell = row->CreateTableCell(50, {2, 2, 2, 2});
        // cell->CreateTextbox(
        //     engine,
        //     "This is a word wrap test with significantly long words.",
        //     24,
        //     TextBox::OverflowBehaviour::WORD_WRAP);
        //
        // cell->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        // cell->tex = window->tex;
        //
        // auto cell2 = row->CreateTableCell();
        // auto cell3 = row->CreateTableCell({0, 0, 5, 0});
        // cell3->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        // cell3->tex = window->tex;
        // auto tex = ResourceManager::GetInstance().TextureLoad("resources/icon.png");
        // auto imgBox = std::make_unique<ImageBox>(engine, cell3, tex);
        // auto imagebox = cell2->CreateImagebox(std::move(imgBox));
        // imagebox->SetGrayscale();
        // auto tex2 = ResourceManager::GetInstance().TextureLoad("resources/icon.png");
        // auto imgBox2 = std::make_unique<ImageBox>(engine, cell3, tex2, VertAlignment::TOP,
        // HoriAlignment::CENTER); auto image2 = cell3->CreateImagebox(std::move(imgBox2)); image2->SetGrayscale();
        //
        // auto row2 = table->CreateTableRow();
        // auto cell4 = row2->CreateTableCell();
        // auto cell5 = row2->CreateTableCell();
        // auto textbox2 = cell4->CreateTextbox(engine, "Bottom Left Alignment");
        // // textbox2->SetVertAlignment(VertAlignment::BOTTOM);
        // // textbox2->SetHoriAlignment(HoriAlignment::LEFT);
        //
        // cell5->CreateTextbox(engine, "This is an example of shrinking!", 42);
    }

    Window* GameUiFactory::CreatePartyPortraitsColumn(GameUIEngine* engine)
    {
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.08;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.3;
        auto _windowDocked = std::make_unique<WindowDocked>(
            engine->gameData->settings,
            0,
            0,
            w,
            h,
            VertAlignment::MIDDLE,
            HoriAlignment::LEFT,
            Padding{12, 12, 16, 16});
        auto* window = engine->CreateWindowDocked(std::move(_windowDocked));
        auto panel = window->CreatePanel();
        auto table = panel->CreateTable();

        auto partySize = engine->gameData->partySystem->GetSize();
        for (int i = partySize - 1; i >= 0; --i) // Do not change "i" to unsigned int.
        {
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell({0, 5, 0, 0});
            auto portrait = std::make_unique<PartyMemberPortrait>(engine, cell, i);
            cell->CreatePartyMemberPortrait(std::move(portrait));
        }
        return window;
    }

    Window* GameUiFactory::CreateAbilityRow(GameUIEngine* engine)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/window_hud.png");

        auto w = 1024; // Absolute value of the image
        auto h = 156;
        auto _windowDocked = std::make_unique<WindowDocked>(
            engine->gameData->settings,
            nPatchTexture,
            TextureStretchMode::SCALE,
            0,
            0,
            w,
            h,
            VertAlignment::BOTTOM,
            HoriAlignment::CENTER,
            Padding{0, 0, 0, 0});
        auto window = engine->CreateWindowDocked(std::move(_windowDocked));
        // window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        auto panel = window->CreatePanel();
        auto table = panel->CreateTable(16);
        auto abilityTable = panel->CreateTable({16, 16, 0, 0});
        panel->CreateTable(16);

        auto experienceBar = abilityTable->CreateTableRow(30);
        auto abilityRow = abilityTable->CreateTableRow(70, {8, 6, 0, 0});
        for (unsigned int i = 0; i < MAX_ABILITY_NUMBER; ++i)
        {
            auto cell = abilityRow->CreateTableCell();
            cell->CreateAbilitySlot(std::make_unique<AbilitySlot>(engine, cell, i));
        }

        // TODO: Currently, if one imagebox has SHRINK_ROW_TO_FIT all imageboxes in that row would be scaled.

        return window;
    }

    TooltipWindow* GameUiFactory::CreateWorldTooltip(GameUIEngine* engine, const std::string& name, Vector2 pos)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.1;
        auto* window = engine->CreateTooltipWindow(
            nullptr, nPatchTexture, TextureStretchMode::NONE, pos.x, pos.y, w, h, {16, 2, 10, 6});
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            auto panel = window->CreatePanel();
            auto table = panel->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::WORD_WRAP;
            auto textbox = std::make_unique<TextBox>(engine, cell0, _fontInfo, VertAlignment::BOTTOM);
            cell0->CreateTextbox(std::move(textbox), name);
        }

        return window;
    }

    TooltipWindow* GameUiFactory::CreateCombatableTooltip(
        GameUIEngine* engine, const std::string& name, CombatableActor& combatInfo, Vector2 pos)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.1;
        auto* window = engine->CreateTooltipWindow(
            nullptr, nPatchTexture, TextureStretchMode::NONE, pos.x, pos.y, w, h, {16, 2, 10, 6});
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        auto panel = window->CreatePanel();
        {
            auto table = panel->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::WORD_WRAP;
            auto headerTextbox = std::make_unique<TextBox>(engine, cell0, _fontInfo, VertAlignment::BOTTOM);
            cell0->CreateTextbox(std::move(headerTextbox), name);
            auto row = table->CreateTableRow({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            auto bodyTextbox = std::make_unique<TextBox>(engine, cell, _fontInfo);
            cell->CreateTextbox(
                std::move(bodyTextbox), std::format("HP: {}/{}", combatInfo.data.hp, combatInfo.data.maxHp));
        }

        return window;
    }

    TooltipWindow* GameUiFactory::CreateItemTooltip(
        GameUIEngine* engine, ItemComponent& item, Window* parentWindow, Vector2 pos)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.1;
        auto* window = engine->CreateTooltipWindow(
            parentWindow, nPatchTexture, TextureStretchMode::NONE, pos.x, pos.y, w, h, {16, 2, 10, 6});
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};

        {
            auto panel = window->CreatePanel();
            auto table = panel->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::WORD_WRAP;
            auto headerTextbox = std::make_unique<TextBox>(engine, cell0, _fontInfo, VertAlignment::BOTTOM);
            cell0->CreateTextbox(std::move(headerTextbox), item.name);
            auto row = table->CreateTableRow({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            auto bodyTextbox = std::make_unique<TextBox>(engine, cell, _fontInfo);
            cell->CreateTextbox(std::move(bodyTextbox), item.description);
        }

        return window;
    }

    TooltipWindow* GameUiFactory::CreateAbilityToolTip(GameUIEngine* engine, const Ability& ability, Vector2 pos)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.10;
        auto* window = engine->CreateTooltipWindow(
            nullptr, nPatchTexture, TextureStretchMode::NONE, pos.x, pos.y, w, h, {16, 2, 10, 6});
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            auto panel = window->CreatePanel();
            auto table = panel->CreateTable();
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

        return window;
    }

    Window* GameUiFactory::CreateInventoryWindow(
        entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto _window = std::make_unique<Window>(
            engine->gameData->settings,
            nPatchTexture,
            TextureStretchMode::SCALE,
            pos.x,
            pos.y,
            274 * 1.5,
            424 * 1.5,
            Padding{20, 0, 14, 14});
        auto window = engine->CreateWindow(std::move(_window));
        entt::sink inventoryUpdateSink{engine->gameData->inventorySystem->onInventoryUpdated};
        {
            auto panel = window->CreatePanel(4);
            auto table = panel->CreateTable();
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell(80);
            auto cell2 = row->CreateTableCell(20);
            auto titleBar = std::make_unique<TitleBar>(engine, cell, TextBox::FontInfo{});
            cell->CreateTitleBar(std::move(titleBar), "Inventory");
            auto tex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_UI_CLOSE);
            auto closeBtn = std::make_unique<CloseButton>(engine, cell2, tex);
            cell2->CreateCloseButton(std::move(closeBtn));
        }

        {
            auto panel1 = window->CreatePanel({20, 0, 0, 0});
            auto table = panel1->CreateTableGrid(INVENTORY_MAX_ROWS, INVENTORY_MAX_COLS, 4);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto& cell = table->children[row]->children[col];
                    auto invSlot = std::make_unique<InventorySlot>(engine, cell.get(), row, col);
                    inventoryUpdateSink.connect<&InventorySlot::RetrieveInfo>(invSlot.get());
                    cell->CreateInventorySlot(std::move(invSlot));
                }
            }
        }
        window->Hide();
        return window;
    }

    Window* GameUiFactory::CreateCharacterWindow(
        entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h)
    {

        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto _window = std::make_unique<Window>(
            engine->gameData->settings,
            nPatchTexture,
            TextureStretchMode::SCALE,
            pos.x,
            pos.y,
            274 * 2,
            424 * 1.5,
            Padding{20, 20, 14, 14});
        auto window = engine->CreateWindow(std::move(_window));
        entt::sink equipmentUpdateSink{engine->gameData->equipmentSystem->onEquipmentUpdated};

        {
            const auto panel1 = window->CreatePanel(4);
            const auto table = panel1->CreateTable();
            const auto row = table->CreateTableRow();
            const auto cell = row->CreateTableCell(80);
            const auto cell2 = row->CreateTableCell(20);
            auto titleText = std::make_unique<TitleBar>(engine, cell, TextBox::FontInfo{});
            cell->CreateTitleBar(std::move(titleText), "Character");

            const auto tex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_UI_CLOSE);
            auto closeBtn = std::make_unique<CloseButton>(engine, cell2, tex);
            cell2->CreateCloseButton(std::move(closeBtn));
        }

        int maxRows = 6;
        int maxCols = 1;

        auto createEquipSlot =
            [&engine, &equipmentUpdateSink](
                const Table* table, unsigned int row, unsigned int col, EquipmentSlotName itemType) {
                auto& cell = table->children[row]->children[col];
                auto equipSlot = std::make_unique<EquipmentSlot>(engine, cell.get(), itemType);
                equipmentUpdateSink.connect<&EquipmentSlot::RetrieveInfo>(equipSlot.get());
                cell->CreateEquipmentSlot(std::move(equipSlot));
            };

        auto createSpacerSlot = [&engine](const Table* table, unsigned int row, unsigned int col) {
            auto& cell = table->children[row]->children[col];
            auto imgBox = std::make_unique<ImageBox>(
                engine, cell.get(), ResourceManager::GetInstance().TextureLoad("resources/transpixel.png"));
            cell->CreateImagebox(std::move(imgBox));
        };

        auto panel2 = window->CreatePanel({28, 0, 24, 24});
        {
            auto table = panel2->CreateTableGrid(maxRows, maxCols, 4);
            for (unsigned int row = 0; row < maxRows; ++row)
            {
                for (unsigned int col = 0; col < maxCols; ++col)
                {
                    createSpacerSlot(table, row, col);
                }
            }

            createEquipSlot(table, 0, 0, EquipmentSlotName::HELM);
            createEquipSlot(table, 1, 0, EquipmentSlotName::ARMS);
            createEquipSlot(table, 2, 0, EquipmentSlotName::CHEST);
            createEquipSlot(table, 3, 0, EquipmentSlotName::AMULET);
            createEquipSlot(table, 4, 0, EquipmentSlotName::LEGS);
            createEquipSlot(table, 5, 0, EquipmentSlotName::LEFTHAND);
        }

        {
            // Character model
            auto table = panel2->CreateTable(60, {24, 24, 24, 24});
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            auto preview = std::make_unique<EquipmentCharacterPreview>(engine, cell);
            auto img = cell->CreateEquipmentCharacterPreview(std::move(preview));
            img->draggable = false;
        }

        {
            auto table = panel2->CreateTableGrid(maxRows, maxCols, 4);

            for (unsigned int row = 0; row < maxRows; ++row)
            {
                for (unsigned int col = 0; col < maxCols; ++col)
                {
                    createSpacerSlot(table, row, col);
                }
            }

            createEquipSlot(table, 0, 0, EquipmentSlotName::BELT);
            createEquipSlot(table, 1, 0, EquipmentSlotName::BOOTS);
            createEquipSlot(table, 2, 0, EquipmentSlotName::RING2);
            createEquipSlot(table, 3, 0, EquipmentSlotName::RING1);
            createEquipSlot(table, 5, 0, EquipmentSlotName::RIGHTHAND);
        }
        window->Hide();
        return window;
    }

    Window* GameUiFactory::CreateDialogWindow(GameUIEngine* engine, entt::entity npc)
    {
        const auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");

        float w = Settings::TARGET_SCREEN_WIDTH * 0.4;
        float h = Settings::TARGET_SCREEN_HEIGHT * 0.25;
        auto _windowDocked = std::make_unique<WindowDocked>(
            engine->gameData->settings,
            nPatchTexture,
            TextureStretchMode::NONE,
            0,
            0,
            w,
            h,
            VertAlignment::BOTTOM,
            HoriAlignment::CENTER,
            Padding{32, 32, 16, 16});
        auto window = engine->CreateWindowDocked(std::move(_windowDocked));
        window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        auto panel = window->CreatePanel();

        auto& dialogComponent = engine->registry->get<DialogComponent>(npc);
        const auto table = panel->CreateTable();
        const auto descriptionRow = table->CreateTableRow();
        const auto descriptionCell = descriptionRow->CreateTableCell({10, 10, 5, 5});

        TextBox::FontInfo _fontInfo;
        _fontInfo.overflowBehaviour = TextBox::OverflowBehaviour::WORD_WRAP;

        auto textbox = std::make_unique<TextBox>(engine, descriptionCell, _fontInfo);
        descriptionCell->CreateTextbox(
            std::move(textbox), dialogComponent.conversation->GetCurrentNode()->content);

        for (auto& o : dialogComponent.conversation->GetCurrentNode()->options)
        {
            const auto optionRow = table->CreateTableRow();
            const auto optionCell = optionRow->CreateTableCell({10, 10, 5, 5});
            auto option = std::make_unique<DialogOption>(engine, optionCell, o, _fontInfo);
            optionCell->CreateDialogOption(std::move(option));
        }
        return window;
    }

    Window* GameUiFactory::CreateGameWindowButtons(
        GameUIEngine* engine, Window* inventoryWindow, Window* equipmentWindow)
    {
        auto w = Settings::TARGET_SCREEN_WIDTH * 0.075;
        auto h = Settings::TARGET_SCREEN_HEIGHT * 0.075;
        auto _windowDocked = std::make_unique<WindowDocked>(
            engine->gameData->settings,
            0,
            0,
            w,
            h,
            VertAlignment::BOTTOM,
            HoriAlignment::LEFT,
            Padding{16, 16, 12, 12});

        auto window = engine->CreateWindowDocked(std::move(_windowDocked));
        window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        auto panel = window->CreatePanel();
        auto table = panel->CreateTable();
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

        return window;
    }
} // namespace sage