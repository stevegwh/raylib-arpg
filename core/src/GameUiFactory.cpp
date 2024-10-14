//
// Created by Steve Wheeler on 03/10/2024.
//

#include "GameUiFactory.hpp"
#include "components/Ability.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "GameUiEngine.hpp"
#include "ResourceManager.hpp"
#include "systems/ControllableActorSystem.hpp"

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

        auto window =
            engine->CreateWindowDocked(nPatchTexture, 0, 0, 40, 20, WindowTableAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPaddingPercent({2, 2, 2, 2});

        auto table = window->CreateTable();

        auto row0 = table->CreateTableRow();
        auto cell0 = row0->CreateTableCell(95);
        cell0->CreateTitleBar(engine, "Title Bar", 12);
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

    void GameUiFactory::CreateAbilityRow(GameUIEngine* engine, PlayerAbilitySystem* playerAbilitySystem)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        {
            auto window =
                engine->CreateWindowDocked(nPatchTexture, 0, 0, 25, 12.5, WindowTableAlignment::STACK_VERTICAL);
            window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
            window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
            window->SetPaddingPercent({5, 2, 2, 2});

            auto table = window->CreateTable();

            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            auto slot = cell->CreateAbilitySlot(engine, playerAbilitySystem, 0);
            // cell->SetPaddingPercent({2, 2, 2, 2});
            auto cell1 = row->CreateTableCell();
            auto slot1 = cell1->CreateAbilitySlot(engine, playerAbilitySystem, 1);
            // cell1->SetPaddingPercent({2, 2, 2, 2});
            auto cell2 = row->CreateTableCell();
            auto slot2 = cell2->CreateAbilitySlot(engine, playerAbilitySystem, 2);
            // cell2->SetPaddingPercent({2, 2, 2, 2});
            auto cell3 = row->CreateTableCell();
            auto slot3 = cell3->CreateAbilitySlot(engine, playerAbilitySystem, 3);
            // cell3->SetPaddingPercent({2, 2, 2, 2});

            // TODO: Currently, if one imagebox has SHRINK_ROW_TO_FIT all imageboxes in that row would be scaled.
            // Is that desired behaviour? Can look for other imageboxes with SHRINK_ROW_TO_FIT as
            // overflowBehaviour?
            slot->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
            slot1->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
            slot2->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
            slot3->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
        }
    }

    // TODO: Combine with CreateAbilityTooltip
    Window* GameUiFactory::CreateItemTooltip(GameUIEngine* engine, ItemComponent& item, Vector2 pos)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto* window =
            engine->CreateWindow(nPatchTexture, pos.x, pos.y, 15, 10, WindowTableAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPaddingPercent({10, 2, 5, 5});

        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            auto textbox = cell0->CreateTextbox(engine, item.name, 10, TextBox::OverflowBehaviour::WORD_WRAP);
            textbox->SetVertAlignment(VertAlignment::BOTTOM);
            auto row = table->CreateTableRow();
            row->SetPaddingPercent({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            cell->CreateTextbox(engine, item.description, 10, TextBox::OverflowBehaviour::WORD_WRAP);
        }

        return window;
    }

    Window* GameUiFactory::CreateAbilityToolTip(GameUIEngine* engine, Ability& ability, Vector2 pos)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        auto* window =
            engine->CreateWindow(nPatchTexture, pos.x, pos.y, 15, 10, WindowTableAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPaddingPercent({10, 2, 5, 5});

        {
            auto table = window->CreateTable();
            auto row0 = table->CreateTableRow(10);
            auto cell0 = row0->CreateTableCell();
            auto textbox = cell0->CreateTextbox(engine, ability.name, 10, TextBox::OverflowBehaviour::WORD_WRAP);
            textbox->SetVertAlignment(VertAlignment::BOTTOM);
            auto row = table->CreateTableRow();
            row->SetPaddingPercent({10, 0, 0, 0});
            auto cell = row->CreateTableCell();
            cell->CreateTextbox(engine, ability.description, 10, TextBox::OverflowBehaviour::WORD_WRAP);
        }

        return window;
    }

    Window* GameUiFactory::CreateInventoryWindow(
        entt::registry* registry,
        GameUIEngine* engine,
        Vector2 pos,
        float w,
        float h,
        ControllableActorSystem* controllableActorSystem)
    {
        auto& inventory = registry->get<InventoryComponent>(controllableActorSystem->GetControlledActor());

        // TODO: Add SetPaddingWindowPercent
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        ResourceManager::GetInstance().ImageLoadFromFile("resources/icon.png");
        auto nPatchTexture = ResourceManager::GetInstance().TextureLoad("resources/textures/ninepatch_button.png");
        // Can populate inventory with ControllableActorSystem where you get the actor's id and get its
        // InventoryComponent

        auto window =
            engine->CreateWindow(nPatchTexture, pos.x, pos.y, w, h, WindowTableAlignment::STACK_VERTICAL);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->SetPaddingPercent({2, 2, 4, 4});

        {
            auto table = window->CreateTable(4);
            auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell(80);
            auto cell2 = row->CreateTableCell(20);
            auto titlebar = cell->CreateTitleBar(engine, "Inventory", 12);
            titlebar->SetHoriAlignment(HoriAlignment::WINDOW_CENTER);
            titlebar->SetVertAlignment(VertAlignment::MIDDLE);
            auto tex = ResourceManager::GetInstance().TextureLoad("resources/icon.png");
            auto closeButton = cell2->CreateCloseButton(engine, tex);
            closeButton->SetHoriAlignment(HoriAlignment::RIGHT);
            closeButton->SetVertAlignment(VertAlignment::TOP);
        }
        {
            auto table = window->CreateTableGrid(INVENTORY_MAX_ROWS, INVENTORY_MAX_COLS, 4);
            for (unsigned int row = 0; row < INVENTORY_MAX_ROWS; ++row)
            {
                for (unsigned int col = 0; col < INVENTORY_MAX_COLS; ++col)
                {
                    auto& cell = table->children[row]->children[col];
                    auto invSlot = cell->CreateInventorySlot(registry, engine, controllableActorSystem, row, col);
                    invSlot->SetOverflowBehaviour(ImageBox::OverflowBehaviour::SHRINK_ROW_TO_FIT);
                }
            }
        }
        window->hidden = true;
        return window;
    }
} // namespace sage