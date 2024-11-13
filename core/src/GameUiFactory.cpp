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
#include "systems/ControllableActorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"

#include <format>

namespace sage
{

    void GameUiFactory::CreateExampleWindow(GameUIEngine* engine)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/icon.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/test.png");

        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        //        info1 = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        //        info2 = {Rectangle{0.0f, 128.0f, 64.0f, 64.0f}, 16, 16, 16, 16, NPATCH_NINE_PATCH};
        //        info3 = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};

        auto window = engine->CreateWindowDocked(nPatchTexture, 0, 0, 40, 20, PanelAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPaddingPercent({2, 2, 2, 2});
        auto panel = window->CreatePanel();
        auto table = panel->CreateTable();

        auto row0 = table->CreateTableRow();
        auto cell0 = row0->CreateTableCell(95);
        cell0->CreateTitleBar(engine, "Title Bar", 24);
        auto cell01 = row0->CreateTableCell();
        auto tex0 = ResourceManager::GetInstance().TextureLoad("resources/icon.png");
        cell01->CreateCloseButton(engine, tex0);

        auto row = table->CreateTableRow(75);
        auto cell = row->CreateTableCell(50);
        cell->CreateTextbox(
            engine,
            "This is a word wrap test with significantly long words.",
            24,
            TextBox::OverflowBehaviour::WORD_WRAP);

        cell->SetPaddingPercent({2, 2, 2, 2});

        cell->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        cell->tex = window->tex;

        auto cell2 = row->CreateTableCell();
        auto cell3 = row->CreateTableCell();
        cell3->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        cell3->tex = window->tex;
        auto tex = ResourceManager::GetInstance().TextureLoad("resources/icon.png");
        auto imagebox = cell2->CreateImagebox(engine, tex);
        imagebox->SetGrayscale();
        // cell2->SetPaddingPercent({10, 10, 10, 10});
        // imagebox->SetHoriAlignment(HoriAlignment::CENTER);
        auto tex2 = ResourceManager::GetInstance().TextureLoad("resources/icon.png");
        auto image2 = cell3->CreateImagebox(engine, tex2);
        image2->SetGrayscale();
        image2->SetHoriAlignment(HoriAlignment::CENTER);

        // auto cell0 = row->CreateTableCell();

        // cell->padding.left = 10;
        // cell2->CreateTextbox("Number 2!");

        // cell3->CreateTextbox("Number 3!");
        cell3->SetPaddingPercent({0, 0, 5, 0});

        auto row2 = table->CreateTableRow();
        auto cell4 = row2->CreateTableCell();
        auto cell5 = row2->CreateTableCell();
        auto textbox2 = cell4->CreateTextbox(engine, "Bottom Left Alignment");
        textbox2->SetVertAlignment(VertAlignment::BOTTOM);
        textbox2->SetHoriAlignment(HoriAlignment::LEFT);

        cell5->CreateTextbox(engine, "This is an example of shrinking!", 42);
    }

    Window* GameUiFactory::CreatePartyPortraitsColumn(GameUIEngine* engine)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/9patch.png");
        // auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");
        auto window = engine->CreateWindowDocked(0, 0, 8, 30, PanelAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::MIDDLE, HoriAlignment::LEFT);
        // window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        window->SetPadding({12, 12, 16, 16});
        auto panel = window->CreatePanel();
        auto table = panel->CreateTable();

        auto partySize = engine->gameData->partySystem->GetSize();
        for (int i = partySize - 1; i >= 0; --i) // Do not change "i" to unsigned int.
        {
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            cell->SetPadding({0, 5, 0, 0});
            auto slot = cell->CreatePartyMemberPortrait(
                engine, engine->gameData->partySystem.get(), engine->gameData->controllableActorSystem.get(), i);
            slot->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
            // TODO: SHRINK_ROW_TO_FIT doesn't work on columns :)
            slot->SetVertAlignment(VertAlignment::TOP);
            slot->SetHoriAlignment(HoriAlignment::CENTER);
        }
        return window;
    }

    Window* GameUiFactory::CreateAbilityRow(GameUIEngine* engine)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/9patch.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/icons/ui/empty.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");

        auto window = engine->CreateWindowDocked(nPatchTexture, 0, 0, 25, 12.5, PanelAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        window->SetPadding({16, 16, 12, 12});
        auto panel = window->CreatePanel();
        auto table = panel->CreateTable();

        auto row = table->CreateTableRow();
        auto cell = row->CreateTableCell();
        auto slot = cell->CreateAbilitySlot(
            engine,
            engine->gameData->playerAbilitySystem.get(),
            engine->gameData->controllableActorSystem.get(),
            0);
        auto cell1 = row->CreateTableCell();
        auto slot1 = cell1->CreateAbilitySlot(
            engine,
            engine->gameData->playerAbilitySystem.get(),
            engine->gameData->controllableActorSystem.get(),
            1);
        auto cell2 = row->CreateTableCell();
        auto slot2 = cell2->CreateAbilitySlot(
            engine,
            engine->gameData->playerAbilitySystem.get(),
            engine->gameData->controllableActorSystem.get(),
            2);
        auto cell3 = row->CreateTableCell();
        auto slot3 = cell3->CreateAbilitySlot(
            engine,
            engine->gameData->playerAbilitySystem.get(),
            engine->gameData->controllableActorSystem.get(),
            3);

        // TODO: Currently, if one imagebox has SHRINK_ROW_TO_FIT all imageboxes in that row would be scaled.
        // Is that desired behaviour? Can look for other imageboxes with SHRINK_ROW_TO_FIT as
        // overflowBehaviour?
        slot->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
        slot1->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
        slot2->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
        slot3->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);

        // TODO: Below seems to do nothing?
        //        slot->SetVertAlignment(VertAlignment::MIDDLE);
        //        slot1->SetVertAlignment(VertAlignment::MIDDLE);
        //        slot2->SetVertAlignment(VertAlignment::MIDDLE);
        //        slot3->SetVertAlignment(VertAlignment::MIDDLE);
        //        slot->SetHoriAlignment(HoriAlignment::CENTER);
        //        slot1->SetHoriAlignment(HoriAlignment::CENTER);
        //        slot2->SetHoriAlignment(HoriAlignment::CENTER);
        //        slot3->SetHoriAlignment(HoriAlignment::CENTER);

        return window;
    }

    Window* GameUiFactory::CreateWorldTooltip(GameUIEngine* engine, const std::string& name, Vector2 pos)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto* window =
            engine->CreateTooltipWindow(nPatchTexture, pos.x, pos.y, 15, 10, PanelAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPadding({10, 2, 5, 5});
        {
            auto panel = window->CreatePanel();
            auto table = panel->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            auto textbox = cell0->CreateTextbox(engine, name, 11, TextBox::OverflowBehaviour::WORD_WRAP);
            textbox->SetVertAlignment(VertAlignment::BOTTOM);
        }

        return window;
    }

    Window* GameUiFactory::CreateCombatableTooltip(
        GameUIEngine* engine, const std::string& name, CombatableActor& combatInfo, Vector2 pos)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto* window =
            engine->CreateTooltipWindow(nPatchTexture, pos.x, pos.y, 15, 10, PanelAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPadding({10, 2, 5, 5});
        auto panel = window->CreatePanel();
        {
            auto table = panel->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            auto textbox = cell0->CreateTextbox(engine, name, 11, TextBox::OverflowBehaviour::WORD_WRAP);
            textbox->SetVertAlignment(VertAlignment::BOTTOM);
            auto row = table->CreateTableRow();
            row->SetPaddingPercent({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            cell->CreateTextbox(
                engine,
                std::format("HP: {}/{}", combatInfo.data.hp, combatInfo.data.maxHp),
                11,
                TextBox::OverflowBehaviour::WORD_WRAP);
        }

        return window;
    }

    Window* GameUiFactory::CreateItemTooltip(GameUIEngine* engine, ItemComponent& item, Vector2 pos)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto* window =
            engine->CreateTooltipWindow(nPatchTexture, pos.x, pos.y, 15, 10, PanelAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPadding({10, 2, 10, 5});
        {
            auto panel = window->CreatePanel();
            auto table = panel->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            auto textbox = cell0->CreateTextbox(engine, item.name, 11, TextBox::OverflowBehaviour::WORD_WRAP);
            textbox->SetVertAlignment(VertAlignment::BOTTOM);
            auto row = table->CreateTableRow();
            row->SetPadding({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            cell->CreateTextbox(engine, item.description, 11, TextBox::OverflowBehaviour::WORD_WRAP);
        }

        return window;
    }

    Window* GameUiFactory::CreateAbilityToolTip(GameUIEngine* engine, const Ability& ability, Vector2 pos)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto* window =
            engine->CreateTooltipWindow(nPatchTexture, pos.x, pos.y, 15, 10, PanelAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPadding({16, 2, 10, 6});
        {
            auto panel = window->CreatePanel();
            auto table = panel->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            auto textbox = cell0->CreateTextbox(engine, ability.name, 11, TextBox::OverflowBehaviour::WORD_WRAP);
            textbox->SetVertAlignment(VertAlignment::BOTTOM);
            auto row = table->CreateTableRow();
            row->SetPadding({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            cell->CreateTextbox(engine, ability.description, 11, TextBox::OverflowBehaviour::WORD_WRAP);
        }

        return window;
    }

    Window* GameUiFactory::CreateInventoryWindow(
        entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/icon.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/9patch.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/icons/ui/empty.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");

        // Can populate inventory with ControllableActorSystem where you get the actor's id and get its
        // InventoryComponent

        auto window = engine->CreateWindow(nPatchTexture, pos.x, pos.y, w, h, PanelAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{3.0f, 665.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        window->SetPadding({14, 14, 14, 14});

        entt::sink inventoryUpdateSink{engine->gameData->inventorySystem->onInventoryUpdated};
        {
            auto panel = window->CreatePanel(4);
            auto table = panel->CreateTable();
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell(80);
            auto cell2 = row->CreateTableCell(20);
            auto titlebar = cell->CreateTitleBar(engine, "Inventory", 15);
            titlebar->SetHoriAlignment(HoriAlignment::WINDOW_CENTER);
            titlebar->SetVertAlignment(VertAlignment::TOP);
            auto tex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_UI_CLOSE);
            auto closeButton = cell2->CreateCloseButton(engine, tex);
            closeButton->SetHoriAlignment(HoriAlignment::RIGHT);
            closeButton->SetVertAlignment(VertAlignment::TOP);
        }

        {
            auto panel1 = window->CreatePanel();
            auto table = panel1->CreateTableGrid(INVENTORY_MAX_ROWS, INVENTORY_MAX_COLS, 4);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto& cell = table->children[row]->children[col];
                    auto invSlot = cell->CreateInventorySlot(
                        engine, engine->gameData->controllableActorSystem.get(), row, col);
                    invSlot->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
                    inventoryUpdateSink.connect<&InventorySlot::RetrieveInfo>(invSlot);
                }
            }
        }
        window->Hide();
        return window;
    }

    Window* GameUiFactory::CreateCharacterWindow(
        entt::registry* registry, GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/transpixel.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/9patch.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");

        auto window = engine->CreateWindow(nPatchTexture, pos.x, pos.y, w, h, PanelAlignment::STACK_HORIZONTAL);
        window->nPatchInfo = {Rectangle{3.0f, 665.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        window->SetPadding({14, 14, 14, 14});

        entt::sink equipmentUpdateSink{engine->gameData->equipmentSystem->onEquipmentUpdated};

        {
            auto panel1 = window->CreatePanel(4);
            auto table = panel1->CreateTable();
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell(80);
            auto cell2 = row->CreateTableCell(20);
            auto titlebar = cell->CreateTitleBar(engine, "Character", 15);
            titlebar->SetHoriAlignment(HoriAlignment::WINDOW_CENTER);
            titlebar->SetVertAlignment(VertAlignment::TOP);
            auto tex = ResourceManager::GetInstance().TextureLoad(AssetID::IMG_UI_CLOSE);
            auto closeButton = cell2->CreateCloseButton(engine, tex);
            closeButton->SetHoriAlignment(HoriAlignment::RIGHT);
            closeButton->SetVertAlignment(VertAlignment::TOP);
        }

        int maxRows = 6;
        int maxCols = 1;

        auto createEquipSlot =
            [&engine, &equipmentUpdateSink](
                const Table* table, unsigned int row, unsigned int col, EquipmentSlotName itemType) {
                auto& cell = table->children[row]->children[col];
                auto equipSlot =
                    cell->CreateEquipmentSlot(engine, engine->gameData->controllableActorSystem.get(), itemType);
                equipSlot->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
                equipmentUpdateSink.connect<&EquipmentSlot::RetrieveInfo>(equipSlot);
            };

        auto createSpacerSlot = [&engine](const Table* table, unsigned int row, unsigned int col) {
            auto& cell = table->children[row]->children[col];
            cell->CreateImagebox(engine, ResourceManager::GetInstance().TextureLoad("resources/transpixel.png"));
        };

        auto panel2 = window->CreatePanel();

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
            auto table = panel2->CreateTable(60);
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            ResourceManager::GetInstance().ImageLoadFromFile("resources/chartest.png");
            auto img = cell->CreateEquipmentCharacterPreview(engine);
            img->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_TO_FIT);
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
        ResourceManager::GetInstance().ImageLoadFromFile("resources/transpixel.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/9patch.png");
        const auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/9patch.png");

        const auto window =
            engine->CreateWindowDocked(nPatchTexture, 0, 0, 40, 25, PanelAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{3.0f, 0.0f, 128.0f, 128.0f}, 32, 12, 32, 12, NPATCH_NINE_PATCH};
        // window->SetOffsetPercent(0, -10);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->SetPadding({32, 32, 16, 16});
        auto panel = window->CreatePanel();

        auto& dialogComponent = engine->registry->get<DialogComponent>(npc);
        const auto table = panel->CreateTable();
        const auto descriptionRow = table->CreateTableRow();
        const auto descriptionCell = descriptionRow->CreateTableCell();
        descriptionCell->SetPadding({10, 10, 5, 5});

        const auto textbox =
            descriptionCell->CreateTextbox(engine, dialogComponent.conversation->GetCurrentNode()->content);
        textbox->SetOverflowBehaviour(TextBox::OverflowBehaviour::WORD_WRAP);

        for (auto& o : dialogComponent.conversation->GetCurrentNode()->options)
        {
            const auto optionRow = table->CreateTableRow();
            const auto optionCell = optionRow->CreateTableCell();
            optionCell->SetPadding({10, 10, 5, 5});
            const auto option = optionCell->CreateDialogOption(engine, o);
            option->SetOverflowBehaviour(TextBox::OverflowBehaviour::WORD_WRAP);
        }
        return window;
    }
} // namespace sage