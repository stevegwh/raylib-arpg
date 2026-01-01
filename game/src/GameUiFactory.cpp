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
#include "components/QuestComponents.hpp"
#include "GameUI.hpp"
#include "QuestManager.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"

#include "engine/AudioManager.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/Cursor.hpp"
#include "engine/ResourceManager.hpp"

#include "magic_enum.hpp"

#include <format>

namespace lq
{

    sage::Window* GameUiFactory::CreatePartyPortraitsColumn(LeverUIEngine* engine)
    {
        auto portraitWidth = 132;
        auto portraitHeight = 166;
        auto w = portraitWidth;
        auto h = portraitHeight * PARTY_MEMBER_MAX;
        auto _windowDocked = std::make_unique<sage::WindowDocked>(
            engine->sys->settings, 16, 16, w, h, sage::VertAlignment::TOP, sage::HoriAlignment::LEFT);
        auto* window = engine->CreateWindowDocked(std::move(_windowDocked));
        auto table = window->CreateTable();

        for (int i = 0; i < PARTY_MEMBER_MAX; ++i)
        {
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            auto portrait = std::make_unique<PartyMemberPortrait>(engine, cell, i, w, 166);
            LeverUIEngine::CreatePartyMemberPortrait(cell, std::move(portrait));
        }

        window->FinalizeLayout();
        return window;
    }

    sage::Window* GameUiFactory::CreateAbilityRow(LeverUIEngine* engine)
    {
        auto nPatchTexture =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/window_hud.png");
        auto healthTex = sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/health.png");
        auto manaTex = sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/mana.png");

        auto w = 1024; // Absolute value of the image
        auto h = 156;
        auto _windowDocked = std::make_unique<sage::WindowDocked>(
            engine->sys->settings,
            nPatchTexture,
            sage::TextureStretchMode::STRETCH,
            0,
            0,
            w,
            h,
            sage::VertAlignment::BOTTOM,
            sage::HoriAlignment::CENTER,
            sage::Padding{0, 0, 0, 0});
        auto window = engine->CreateWindowDocked(std::move(_windowDocked));

        auto tableMain = window->CreateTable();
        auto tableMainRow = tableMain->CreateTableRow();

        auto healthCell = tableMainRow->CreateTableCell(15.5, {8, 8, 8, 8});
        healthCell->CreateImagebox(std::make_unique<sage::ImageBox>(engine, healthCell, healthTex));

        auto abilityCell = tableMainRow->CreateTableCell({16, 0, 0, 0});
        auto manaCell = tableMainRow->CreateTableCell(15.5, {8, 8, 8, 8});
        manaCell->CreateImagebox(std::make_unique<sage::ImageBox>(engine, manaCell, manaTex));

        auto table = abilityCell->CreateTable();
        table->CreateTableRow(24); // Experience bar
        auto abilityRow = table->CreateTableRowGrid(MAX_ABILITY_NUMBER, 4, {4, 4, 0, 0});
        for (unsigned int i = 0; i < abilityRow->children.size(); ++i)
        {
            auto cell = dynamic_cast<sage::TableCell*>(abilityRow->children[i].get());
            LeverUIEngine::CreateAbilitySlot(cell, std::make_unique<AbilitySlot>(engine, cell, i));
        }

        // TODO: Currently, if one imagebox has SHRINK_ROW_TO_FIT all imageboxes in that row would be scaled.
        window->FinalizeLayout();
        return window;
    }

    sage::TooltipWindow* GameUiFactory::CreateWorldTooltip(
        LeverUIEngine* engine, const std::string& name, Vector2 pos)
    {

        // Set window's dimensions to the size of the text
        sage::Padding padding = {10, 10, 10, 20};

        sage::TextBox::FontInfo _fontInfo{};
        _fontInfo.overflowBehaviour = sage::TextBox::OverflowBehaviour::SHRINK_TO_FIT;
        float scaleFactor = engine->sys->settings->GetCurrentScaleFactor();
        _fontInfo.fontSize = _fontInfo.baseFontSize * scaleFactor;
        _fontInfo.fontSize = std::clamp(
            _fontInfo.fontSize, sage::TextBox::FontInfo::minFontSize, sage::TextBox::FontInfo::maxFontSize);

        Vector2 textSize = MeasureTextEx(_fontInfo.font, name.c_str(), _fontInfo.fontSize, _fontInfo.fontSpacing);
        auto w = textSize.x + padding.left + padding.right;
        auto h = textSize.y + padding.up + padding.down;

        auto nPatchTexture =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");

        auto tooltip = std::make_unique<sage::TooltipWindow>(
            engine->sys->settings,
            nullptr,
            nPatchTexture,
            sage::TextureStretchMode::NONE,
            pos.x,
            pos.y,
            w,
            h,
            padding);

        auto* window = engine->CreateTooltipWindow(std::move(tooltip));

        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();

            auto textbox = std::make_unique<sage::TextBox>(engine, cell0, _fontInfo);
            cell0->CreateTextbox(std::move(textbox), name);
        }

        window->FinalizeLayout();
        return window;
    }

    sage::TooltipWindow* GameUiFactory::CreateItemTooltip(
        sage::GameUIEngine* engine, const ItemComponent& item, sage::Window* parentWindow, Vector2 pos)
    {
        auto nPatchTexture =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = sage::Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = sage::Settings::TARGET_SCREEN_HEIGHT * 0.1;
        auto tooltip = std::make_unique<sage::TooltipWindow>(
            engine->settings,
            parentWindow,
            nPatchTexture,
            sage::TextureStretchMode::NONE,
            pos.x,
            pos.y,
            w,
            h,
            sage::Padding{20, 20, 10, 6});
        auto* window = engine->CreateTooltipWindow(std::move(tooltip));

        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            sage::TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = sage::TextBox::OverflowBehaviour::WORD_WRAP;
            auto headerTextbox =
                std::make_unique<sage::TextBox>(engine, cell0, _fontInfo, sage::VertAlignment::BOTTOM);
            cell0->CreateTextbox(std::move(headerTextbox), item.localizedName);
            auto row = table->CreateTableRow({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            auto bodyTextbox = std::make_unique<sage::TextBox>(engine, cell, _fontInfo);
            cell->CreateTextbox(std::move(bodyTextbox), item.description);
        }
        window->FinalizeLayout();
        return window;
    }

    sage::TooltipWindow* GameUiFactory::CreateAbilityToolTip(
        sage::GameUIEngine* engine, const Ability& ability, Vector2 pos)
    {
        auto nPatchTexture =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto w = sage::Settings::TARGET_SCREEN_WIDTH * 0.15;
        auto h = sage::Settings::TARGET_SCREEN_HEIGHT * 0.1;
        auto tooltip = std::make_unique<sage::TooltipWindow>(
            engine->settings,
            nullptr,
            nPatchTexture,
            sage::TextureStretchMode::NONE,
            pos.x,
            pos.y,
            w,
            h,
            sage::Padding{20, 20, 10, 6});
        auto* window = engine->CreateTooltipWindow(std::move(tooltip));

        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        {
            const auto& ad = engine->registry->get<AbilityData>(ability.self);
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            sage::TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = sage::TextBox::OverflowBehaviour::WORD_WRAP;
            auto headerTextbox =
                std::make_unique<sage::TextBox>(engine, cell0, _fontInfo, sage::VertAlignment::BOTTOM);
            cell0->CreateTextbox(std::move(headerTextbox), ad.name);
            auto row = table->CreateTableRow({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            auto bodyTextbox = std::make_unique<sage::TextBox>(engine, cell, _fontInfo);
            cell->CreateTextbox(std::move(bodyTextbox), ad.description);
        }
        window->FinalizeLayout();
        return window;
    }

    sage::Window* GameUiFactory::CreateLootWindow(
        entt::registry* registry, LeverUIEngine* engine, entt::entity owner, Vector2 pos)
    {
        auto nPatchTexture =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/inventory-bg.png");
        // TODO: Have a special "loot window" which overrides Update and if the player moves out of loot range then
        // it closes itself.
        auto _window = std::make_unique<sage::Window>(
            engine->sys->settings,
            nPatchTexture,
            sage::TextureStretchMode::STRETCH,
            pos.x,
            pos.y,
            274,
            280,
            sage::Padding{6, 6, 2, 2});
        auto window = engine->CreateWindow(std::move(_window));
        auto mainTable = window->CreateTable();
        auto mainTableRow1 = mainTable->CreateTableRow(8);
        auto mainTableRow2 = mainTable->CreateTableRow({8, 4, 0, 0});

        {
            // Title bar
            auto cell = mainTableRow1->CreateTableCell(80);
            auto cell2 = mainTableRow1->CreateTableCell(20);
            auto titleBar = std::make_unique<sage::TitleBar>(engine, cell, sage::TextBox::FontInfo{});
            cell->CreateTitleBar(std::move(titleBar), "");
            auto tex = sage::ResourceManager::GetInstance().TextureLoad("ui_close");
            auto closeBtn = std::make_unique<sage::CloseButton>(engine, cell2, tex, true);
            cell2->CreateCloseButton(std::move(closeBtn));
            // TODO: Close should DELETE.
        }

        {
            // Inventory grid
            auto cell = mainTableRow2->CreateTableCell();
            auto table = cell->CreateTableGrid(3, 3, 4);
            for (unsigned int row = 0; row < 3; ++row)
            {
                for (unsigned int col = 0; col < 3; ++col)
                {
                    auto invCell = dynamic_cast<sage::TableCell*>(table->children[row]->children[col].get());
                    auto invSlot = std::make_unique<InventorySlot>(engine, invCell, owner, row, col);
                    LeverUIEngine::CreateInventorySlot(invCell, std::move(invSlot));
                }
            }
        }
        window->FinalizeLayout();
        return window;
    }

    sage::Window* GameUiFactory::CreateInventoryWindow(
        entt::registry* registry, LeverUIEngine* engine, Vector2 pos, float w, float h)
    {
        auto nPatchTexture = sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto _window = std::make_unique<sage::Window>(
            engine->sys->settings,
            nPatchTexture,
            sage::TextureStretchMode::STRETCH,
            pos.x,
            pos.y,
            274 * 1.5,
            424 * 1.5,
            sage::Padding{20, 0, 14, 14});
        auto window = engine->CreateWindow(std::move(_window));
        auto mainTable = window->CreateTable();
        auto mainTableRow1 = mainTable->CreateTableRow(4);
        auto mainTableRow2 = mainTable->CreateTableRow({20, 0, 0, 0});

        {
            // Title bar
            auto cell = mainTableRow1->CreateTableCell(80);
            auto cell2 = mainTableRow1->CreateTableCell(20);
            auto titleBar = std::make_unique<sage::TitleBar>(engine, cell, sage::TextBox::FontInfo{});
            cell->CreateTitleBar(std::move(titleBar), "Inventory");
            auto tex = sage::ResourceManager::GetInstance().TextureLoad("ui_close");
            auto closeBtn = std::make_unique<sage::CloseButton>(engine, cell2, tex);
            cell2->CreateCloseButton(std::move(closeBtn));
        }

        {
            auto actor = engine->sys->cursor->GetSelectedActor();
            // Inventory grid
            auto cell = mainTableRow2->CreateTableCell();
            auto table = cell->CreateTableGrid(INVENTORY_MAX_ROWS, INVENTORY_MAX_COLS, 4);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto invCell = dynamic_cast<sage::TableCell*>(table->children[row]->children[col].get());
                    auto invSlot = std::make_unique<InventorySlot>(engine, invCell, actor, row, col);
                    auto ptr = LeverUIEngine::CreateInventorySlot(invCell, std::move(invSlot));

                    engine->sys->cursor->onSelectedActorChange.Subscribe(
                        [ptr](entt::entity prev, entt::entity current) { ptr->SetOwner(current); });

                    engine->sys->inventorySystem->onInventoryUpdated.Subscribe([ptr]() { ptr->RetrieveInfo(); });
                }
            }
        }
        window->FinalizeLayout();
        window->Hide();

        window->onShow.Subscribe(
            [engine]() { engine->sys->audioManager->PlaySFX("resources/audio/sfx/inv_open.ogg"); });
        window->onHide.Subscribe(
            [engine]() { engine->sys->audioManager->PlaySFX("resources/audio/sfx/inv_close.ogg"); });

        return window;
    }

    sage::Window* GameUiFactory::CreateJournalWindow(
        entt::registry* registry, LeverUIEngine* engine, Vector2 pos, float w, float h)
    {
        auto nPatchTexture = sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto _window = std::make_unique<sage::Window>(
            engine->sys->settings,
            nPatchTexture,
            sage::TextureStretchMode::STRETCH,
            pos.x,
            pos.y,
            274 * 3,
            424 * 1.5,
            sage::Padding{20, 20, 14, 14});

        auto window = engine->CreateWindow(std::move(_window));
        auto mainTable = window->CreateTable({0, 0, 4, 0});

        {
            const auto titleRow = mainTable->CreateTableRow(10);
            const auto cell = titleRow->CreateTableCell(80);
            const auto cell2 = titleRow->CreateTableCell(20, {0, 18 * 2, 18, 18});
            auto titleText = std::make_unique<sage::TitleBar>(engine, cell, sage::TextBox::FontInfo{});
            cell->CreateTitleBar(std::move(titleText), "Journal");

            const auto tex = sage::ResourceManager::GetInstance().TextureLoad("ui_close");
            auto closeBtn = std::make_unique<sage::CloseButton>(engine, cell2, tex);
            cell2->CreateCloseButton(std::move(closeBtn));
        }

        auto mainRow = mainTable->CreateTableRow();
        auto questList = mainRow->CreateTableCell({48, 0, 8, 8});
        auto questDescription = mainRow->CreateTableCell(70, {48, 12, 24, 24});

        mainRow->SetTexture(
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/window_quest.png"),
            sage::TextureStretchMode::STRETCH);
        questList->SetTexture(
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/quest-bg.png"),
            sage::TextureStretchMode::STRETCH);
        questDescription->SetTexture(
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/quest-bg.png"),
            sage::TextureStretchMode::STRETCH);

        {
            // The 'description' textbox also acts as a manager for the quests shown in the sidebar
            sage::TextBox::FontInfo _fontInfo{};
            _fontInfo.overflowBehaviour = sage::TextBox::OverflowBehaviour::WORD_WRAP;
            auto journalEntryManager = std::make_unique<JournalEntryManager>(
                engine,
                questDescription,
                questList,
                engine->sys->questManager.get(),
                _fontInfo,
                sage::VertAlignment::TOP,
                sage::HoriAlignment::CENTER);
            questDescription->element = std::move(journalEntryManager);
        }

        window->FinalizeLayout();
        window->Hide();
        window->onShow.Subscribe(
            [engine]() { engine->sys->audioManager->PlaySFX("resources/audio/sfx/book_open.ogg"); });
        window->onHide.Subscribe(
            [engine]() { engine->sys->audioManager->PlaySFX("resources/audio/sfx/inv_close.ogg"); });

        return window;
    }

    sage::Window* GameUiFactory::CreateCharacterWindow(
        entt::registry* registry, LeverUIEngine* engine, Vector2 pos, float w, float h)
    {
        auto nPatchTexture = sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/frame.png");
        auto _window = std::make_unique<sage::Window>(
            engine->sys->settings,
            nPatchTexture,
            sage::TextureStretchMode::STRETCH,
            pos.x,
            pos.y,
            274 * 3,
            424 * 1.5,
            sage::Padding{20, 20, 14, 14});

        auto window = engine->CreateWindow(std::move(_window));
        auto mainTable = window->CreateTable({0, 0, 4, 0});

        {
            // TODO: Having the concept of "margins" would make it so much easier to create consistent layouts
            // where you don't have to request different heights and add padding to make things line up
            const auto titleRow = mainTable->CreateTableRow(10);
            const auto cell = titleRow->CreateTableCell(80);
            const auto cell2 = titleRow->CreateTableCell(20, {0, 18 * 2, 18, 18});
            auto titleText = std::make_unique<sage::TitleBar>(engine, cell, sage::TextBox::FontInfo{});
            cell->CreateTitleBar(std::move(titleText), "Character");

            const auto tex = sage::ResourceManager::GetInstance().TextureLoad("ui_close");
            auto closeBtn = std::make_unique<sage::CloseButton>(engine, cell2, tex);
            cell2->CreateCloseButton(std::move(closeBtn));
        }

        int maxRows = 6;
        int maxCols = 1;

        auto createEquipSlot =
            [&engine](const sage::Table* table, unsigned int row, unsigned int col, EquipmentSlotName itemType) {
                auto cell = dynamic_cast<sage::TableCell*>(table->children[row]->children[col].get());
                auto equipSlot = std::make_unique<EquipmentSlot>(engine, cell, itemType);
                auto* ptr = LeverUIEngine::CreateEquipmentSlot(cell, std::move(equipSlot));
                engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe(
                    [ptr](entt::entity) { ptr->RetrieveInfo(); });
            };

        auto createSpacerSlot = [&engine](const sage::Table* table, unsigned int row, unsigned int col) {
            auto cell = dynamic_cast<sage::TableCell*>(table->children[row]->children[col].get());
            auto imgBox = std::make_unique<sage::ImageBox>(
                engine, cell, sage::ResourceManager::GetInstance().TextureLoad("resources/transpixel.png"));
            cell->CreateImagebox(std::move(imgBox));
        };

        auto mainRow = mainTable->CreateTableRow({0, 0, 4, 0});
        mainRow->SetTexture(
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/inventory-bg.png"),
            sage::TextureStretchMode::STRETCH);

        const auto charColLeft = mainRow->CreateTableCell();
        const auto charColCenter = mainRow->CreateTableCell(40, {0, 0, 24, 24});
        const auto charColRight = mainRow->CreateTableCell();
        const auto statsCol = mainRow->CreateTableCell(40, {50, 250, 48, 48});
        statsCol->SetTexture(
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/scroll-bg.png"),
            sage::TextureStretchMode::STRETCH);

        {
            // Left items
            auto table = charColLeft->CreateTableGrid(maxRows, maxCols, 4);
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
            auto table = charColCenter->CreateTable();
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            auto preview = std::make_unique<EquipmentCharacterPreview>(engine, cell);
            auto img = LeverUIEngine::CreateEquipmentCharacterPreview(cell, std::move(preview));
            img->draggable = false;
        }

        {
            // Right items
            auto table = charColRight->CreateTableGrid(maxRows, maxCols, 4);

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

        {
            // Character statistics
            auto table = statsCol->CreateTable();
            // ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ui/window_dialogue.png");

            for (int i = 0; i < magic_enum::enum_underlying(CharacterStatText::StatisticType::COUNT); ++i)
            {
                auto row = table->CreateTableRow();
                auto cell = row->CreateTableCell();
                auto id = magic_enum::enum_cast<CharacterStatText::StatisticType>(i).value();
                auto stat = std::make_unique<CharacterStatText>(engine, cell, sage::TextBox::FontInfo{}, id);
                LeverUIEngine::CreateCharacterStatText(cell, std::move(stat));
            }
        }

        window->FinalizeLayout();
        window->Hide();
        window->onShow.Subscribe(
            [engine]() { engine->sys->audioManager->PlaySFX("resources/audio/sfx/equip_open.ogg"); });
        window->onHide.Subscribe(
            [engine]() { engine->sys->audioManager->PlaySFX("resources/audio/sfx/inv_close.ogg"); });

        return window;
    }

    sage::Window* GameUiFactory::CreateDialogWindow(LeverUIEngine* engine, entt::entity npc)
    {
        const auto nPatchTexture =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");

        float w = sage::Settings::TARGET_SCREEN_WIDTH * 0.65;
        float h = sage::Settings::TARGET_SCREEN_HEIGHT * 0.35;

        auto _windowDocked = std::make_unique<sage::WindowDocked>(
            engine->sys->settings,
            nPatchTexture,
            sage::TextureStretchMode::NONE,
            0,
            0,
            w,
            h,
            sage::VertAlignment::BOTTOM,
            sage::HoriAlignment::CENTER,
            sage::Padding{32, 32, 16, 16});

        auto window = engine->CreateWindowDocked(std::move(_windowDocked));
        window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};

        auto& dialogComponent = engine->registry->get<DialogComponent>(npc);
        sage::TextBox::FontInfo _fontInfo;
        _fontInfo.baseFontSize = 20;
        _fontInfo.overflowBehaviour = sage::TextBox::OverflowBehaviour::WORD_WRAP;

        const auto table = window->CreateTable();
        // NPC Part
        {
            auto& renderable = engine->registry->get<sage::Renderable>(npc);
            const auto descriptionRow = table->CreateTableRow();
            const auto descriptionCell = descriptionRow->CreateTableCell({10, 10, 20, 20});
            auto textbox = std::make_unique<sage::TextBox>(engine, descriptionCell, _fontInfo);

            std::string speakerName;
            if (!dialogComponent.conversation->speaker.empty())
            {
                speakerName = dialogComponent.conversation->speaker;
            }
            else
            {
                speakerName = renderable.GetVanityName();
            }

            descriptionCell->CreateTextbox(
                std::move(textbox),
                std::format("[{}]: {}", speakerName, dialogComponent.conversation->GetCurrentNode()->content));
        }
        // ----------------

        // Player part
        {
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            const auto portraitTable = cell->CreateTable();
            auto portraitRow = portraitTable->CreateTableRow();
            auto portraitCell = portraitRow->CreateTableCell();

            const auto& info =
                engine->registry->get<PartyMemberComponent>(engine->sys->cursor->GetSelectedActor());
            auto tex = info.portraitImg.texture;
            auto img = std::make_unique<DialogPortrait>(engine, portraitCell, tex);
            portraitCell->CreateImagebox(std::move(img));

            auto cell2 = row->CreateTableCell(80);
            const auto optionsTable = cell2->CreateTable();
            unsigned int i = 0;
            for (const auto& o : dialogComponent.conversation->GetCurrentNode()->options)
            {
                if (!o->ShouldShow()) continue;
                const auto optionRow = optionsTable->CreateTableRow();
                const auto optionCell = optionRow->CreateTableCell({10, 10, 10, 10});
                auto option = std::make_unique<DialogOption>(engine, optionCell, o.get(), ++i, _fontInfo);
                LeverUIEngine::CreateDialogOption(optionCell, std::move(option));
            }
        }
        window->FinalizeLayout();
        return window;
    }

    sage::Window* GameUiFactory::CreateGameWindowButtons(
        LeverUIEngine* engine,
        sage::Window* inventoryWindow,
        sage::Window* equipmentWindow,
        sage::Window* journalWindow)
    {
        auto w = sage::Settings::TARGET_SCREEN_WIDTH * 0.1;
        auto h = sage::Settings::TARGET_SCREEN_HEIGHT * 0.075;
        auto _windowDocked = std::make_unique<sage::WindowDocked>(
            engine->sys->settings,
            0,
            0,
            w,
            h,
            sage::VertAlignment::BOTTOM,
            sage::HoriAlignment::LEFT,
            sage::Padding{16, 16, 12, 12});

        auto window = engine->CreateWindowDocked(std::move(_windowDocked));
        window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        auto table = window->CreateTable();
        auto row = table->CreateTableRow();
        auto cell = row->CreateTableCell();
        cell->CreateGameWindowButton(
            std::make_unique<sage::GameWindowButton>(
                engine,
                cell,
                sage::ResourceManager::GetInstance().TextureLoad("resources/icons/ui/inventory.png"),
                inventoryWindow));
        auto cell1 = row->CreateTableCell();
        cell1->CreateGameWindowButton(
            std::make_unique<sage::GameWindowButton>(
                engine,
                cell1,
                sage::ResourceManager::GetInstance().TextureLoad("resources/icons/ui/equipment.png"),
                equipmentWindow));
        auto cell2 = row->CreateTableCell();
        cell2->CreateGameWindowButton(
            std::make_unique<sage::GameWindowButton>(
                engine,
                cell2,
                sage::ResourceManager::GetInstance().TextureLoad("resources/icons/ui/spellbook.png"),
                journalWindow));
        window->FinalizeLayout();
        return window;
    }
} // namespace lq