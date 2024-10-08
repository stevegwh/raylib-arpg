//
// Created by Steve Wheeler on 03/10/2024.
//

#include "GameUiFactory.hpp"
#include "GameUiEngine.hpp"
#include "ResourceManager.hpp"
#include "systems/PlayerAbilitySystem.hpp"

namespace sage
{

    void GameUiFactory::CreateExampleWindow(GameUIEngine* engine)
    {
        // TODO: Have a "CreateTitleBar" with a textbox and a image of a cross for closing
        // When the title bar is clicked, you can have it get the parent's window pos and update it based on mouse
        // location (and call update children)

        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().GetImage("resources/textures/ninepatch_button.png");
        //        info1 = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        //        info2 = {Rectangle{0.0f, 128.0f, 64.0f, 64.0f}, 16, 16, 16, 16, NPATCH_NINE_PATCH};
        //        info3 = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};

        auto window =
            engine->CreateWindow(nPatchTexture.GetImage(), 0, 0, 40, 20, WindowTableAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->tex = window->mainNPatchTexture;
        window->SetPaddingPercent({2, 2, 2, 2});

        auto table = window->CreateTable();

        auto row0 = table->CreateTableRow();
        auto cell0 = row0->CreateTableCell(95);
        cell0->CreateTitleBar(window, "Title Bar", 12);
        auto cell01 = row0->CreateTableCell();
        cell01->CreateCloseButton(window, LoadImage("resources/icon.png"));

        auto row = table->CreateTableRow(75);
        auto cell = row->CreateTableCell(50);
        cell->CreateTextbox(
            "This is a word wrap test with significantly long words.", 24, TextBox::OverflowBehaviour::WORD_WRAP);

        cell->SetPaddingPercent({2, 2, 2, 2});

        cell->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        cell->tex = window->mainNPatchTexture;

        auto cell2 = row->CreateTableCell();
        auto cell3 = row->CreateTableCell();
        cell3->nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        cell3->tex = window->mainNPatchTexture;
        auto imagebox = cell2->CreateImagebox(window, LoadImage("resources/icon.png"));
        imagebox->SetGrayscale();
        // cell2->SetPaddingPercent({10, 10, 10, 10});
        // imagebox->SetHoriAlignment(HoriAlignment::CENTER);
        auto image2 = cell3->CreateImagebox(window, LoadImage("resources/test.png"));
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
        auto textbox2 = cell4->CreateTextbox("Bottom Left Alignment");
        textbox2->SetVertAlignment(VertAlignment::BOTTOM);
        textbox2->SetHoriAlignment(HoriAlignment::LEFT);

        cell5->CreateTextbox("This is an example of shrinking!", 42);
    }

    void GameUiFactory::CreateAbilityRow(GameUIEngine* engine, PlayerAbilitySystem* playerAbilitySystem)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/textures/ninepatch_button.png");
        auto nPatchTexture = ResourceManager::GetInstance().GetImage("resources/textures/ninepatch_button.png");

        // TODO: Bug: Hover does not disappear when you go directly from one image to another
        // TODO: Bug: Only one image gets scaled down to fit in the row. If you want all images to be the same
        // scale then it doesn't work as intended
        auto window =
            engine->CreateWindow(nPatchTexture.GetImage(), 0, 0, 40, 20, WindowTableAlignment::STACK_VERTICAL);
        window->SetAlignment(VertAlignment::BOTTOM, HoriAlignment::CENTER);
        window->nPatchInfo = {Rectangle{0.0f, 64.0f, 64.0f, 64.0f}, 8, 8, 8, 8, NPATCH_NINE_PATCH};
        window->tex = window->mainNPatchTexture;
        window->SetPaddingPercent({5, 2, 2, 2});

        auto table = window->CreateTable();

        auto row = table->CreateTableRow();
        auto cell = row->CreateTableCell();
        auto slot = cell->CreateAbilitySlot(playerAbilitySystem, window, 0);
        // cell->SetPaddingPercent({2, 2, 2, 2});
        auto cell1 = row->CreateTableCell();
        auto slot1 = cell1->CreateAbilitySlot(playerAbilitySystem, window, 1);
        // cell1->SetPaddingPercent({2, 2, 2, 2});
        auto cell2 = row->CreateTableCell();
        auto slot2 = cell2->CreateAbilitySlot(playerAbilitySystem, window, 2);
        // cell2->SetPaddingPercent({2, 2, 2, 2});
        auto cell3 = row->CreateTableCell();
        auto slot3 = cell3->CreateAbilitySlot(playerAbilitySystem, window, 3);
        // cell3->SetPaddingPercent({2, 2, 2, 2});

        {
            entt::sink sink{slot->onMouseClicked};
            sink.connect<&PlayerAbilitySystem::AbilityOnePressed>(playerAbilitySystem);
            entt::sink sink1{slot1->onMouseClicked};
            sink1.connect<&PlayerAbilitySystem::AbilityTwoPressed>(playerAbilitySystem);
            entt::sink sink2{slot2->onMouseClicked};
            sink2.connect<&PlayerAbilitySystem::AbilityThreePressed>(playerAbilitySystem);
            entt::sink sink3{slot3->onMouseClicked};
            sink3.connect<&PlayerAbilitySystem::AbilityFourPressed>(playerAbilitySystem);
        }
    }

    void GameUiFactory::CreateInventoryWindow(GameUIEngine* engine, Vector2 pos, float w, float h)
    {
        // engine->CreateWindow(pos, w, h);
    }
} // namespace sage